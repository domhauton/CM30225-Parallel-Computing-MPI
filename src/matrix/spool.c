//
// Created by dominic on 19/11/16.
//

#include "spool.h"
#include "../debug.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <zconf.h>

/* FIFO Job Queue and LIFO thread stacks */
typedef struct SPOOL_T {
    spool_job_t *nextJob;
    spool_job_t *lastJob;
    sem_t jobWaitSem;
    pthread_spinlock_t jobQueueAccessLock;

    spool_thread_t *threads;
    spool_thread_t *threadGraveyard;
    pthread_spinlock_t threadAccessLock;
} spool_t;

typedef struct SPOOL_THREAD_PARAMS_T {
    spool_t *smoothPool;
    bool running;
} spool_thread_params_t;

typedef struct SPOOL_THREAD_T {
    pthread_t thread;
    spool_thread_params_t *threadParams;
    spool_thread_t *next;
} spool_thread_t;

typedef struct SPOOL_JOB_SYNC_T {
    pthread_cond_t endOfJob;
    pthread_mutex_t countLock;
    unsigned int count;
} spool_job_sync_t;

typedef struct SPOOL_JOB_T {
    mat_smthr_t *matSmoother;
    spool_job_t *next;
    spool_job_sync_t *spoolJobSync;
} spool_job_t;

void spool_job_add_inner(spool_t *spool, spool_job_t* newSpoolJob) {
    newSpoolJob->next = NULL;
    pthread_spin_lock(&spool->jobQueueAccessLock);
    if (spool->nextJob == NULL) {
        spool->nextJob = newSpoolJob;
    } else {
        spool->lastJob->next = newSpoolJob;
    }
    spool->lastJob = newSpoolJob;
    pthread_spin_unlock(&spool->jobQueueAccessLock);
}

void spool_job_add(spool_t *spool, mat_smthr_t *smoother, spool_job_sync_t *spoolJobSync) {
    debug_print("Smoothing Pool\t- DISPATCHER   - Job Add\n");
    spool_job_t *newSpoolJob = malloc(sizeof(spool_job_t));
    newSpoolJob->matSmoother = smoother;
    newSpoolJob->spoolJobSync = spoolJobSync;
    spool_job_add_inner(spool, newSpoolJob);
    sem_post(&spool->jobWaitSem);
}

/* Creates a sync that allows jobs to indicate completion and the main thread to wait */
spool_job_sync_t *spool_job_sync_init(unsigned int count) {
    spool_job_sync_t *spool_job_sync = malloc(sizeof(spool_job_sync_t));
    pthread_mutex_init(&spool_job_sync->countLock, NULL);
    pthread_cond_init(&spool_job_sync->endOfJob, NULL);
    spool_job_sync->count = count;
    return spool_job_sync;
}

/* Add a set of jobs to be processed.
        Returns a spool_job_sync_t for synchronisation.
        Destroys list items after processing. */
spool_job_sync_t *spool_job_add_batch(spool_t *spool, mat_smthr_list_t *smthrList, unsigned int count) {
    debug_print("Smoothing Pool\t- DISPATCHER   - Submitted %d job/s.\n", count);
    spool_job_sync_t *jobSync = spool_job_sync_init(count);
    while (smthrList != NULL) {
        mat_smthr_list_t *currentSmoothSection = smthrList;
        spool_job_add(spool, currentSmoothSection->data, jobSync);
        smthrList = smthrList->next;
        free(currentSmoothSection);
    }
    return jobSync;
}

/* Get current job and increment job pointer.
        Returns NULL if no job available.      */
spool_job_t *spool_job_next(spool_t *smoothPool) {
    debug_print("Smoothing Pool\t- Thread %ld - Job Fetched\n", syscall(__NR_gettid));
    spool_job_t *retJob;
    pthread_spin_lock(&smoothPool->jobQueueAccessLock);
    retJob = smoothPool->nextJob;
    if (retJob != NULL) {
        smoothPool->nextJob = retJob->next;
    }
    pthread_spin_unlock(&smoothPool->jobQueueAccessLock);
    return retJob;
}

/* Wait until jobs available in pool.
        Return job when available.      */
spool_job_t *spool_job_wait(spool_thread_params_t *smoothThread) {
    debug_print("Smoothing Pool\t- Thread %ld - Job Wait\n", syscall(__NR_gettid));
    sem_wait(&smoothThread->smoothPool->jobWaitSem);
    spool_job_t* skipJob = spool_job_next(smoothThread->smoothPool);
    if(skipJob != NULL){
        spool_job_add_inner(smoothThread->smoothPool, skipJob);
    }
    return spool_job_next(smoothThread->smoothPool);
}

/* Decrements the remaining job counter by one */
void spool_job_sync_complete(spool_job_sync_t *spoolJobSync) {
    pthread_mutex_lock(&spoolJobSync->countLock);
    spoolJobSync->count--;
    debug_print("Smoothing Pool\t- Thread %ld - Signalled job complete. New count: %d\n", syscall(__NR_gettid), spoolJobSync->count);
    pthread_cond_signal(&spoolJobSync->endOfJob);
    pthread_mutex_unlock(&spoolJobSync->countLock);
}

/* Fetch jobs and execute jobs while running bool is true*/
void *spool_thread_run(void *voidArgs) {
    debug_print("Smoothing Pool\t- Thread %ld - Worker Initialised\n", syscall(__NR_gettid));
    spool_thread_params_t *smoothThread = voidArgs;
    while (smoothThread->running) {
        spool_job_t *smoothJob = spool_job_wait(smoothThread);
        if (smoothJob != NULL) {
            if (smoothJob->matSmoother != NULL) {
                mat_smthr_smooth(smoothJob->matSmoother);
                mat_smthr_destroy(smoothJob->matSmoother);
            }
            spool_job_sync_complete(smoothJob->spoolJobSync);
            debug_print("Smoothing Pool\t- Thread %ld - Job Complete (Smoothing)\n", syscall(__NR_gettid));
            free(smoothJob);
        } else {
            debug_print("Smoothing Pool\t- Thread %ld - Job Complete (Dummy)\n", syscall(__NR_gettid));
        }
    }
    return NULL;
}

/* Adds a new worker thread to the smoothing thread pool */
void spool_worker_add(spool_t *spool) {
    debug_print("Smoothing Pool\t- Adding Worker Start\n");
    spool_thread_t *newSpoolThread = malloc(sizeof(spool_thread_t));
    newSpoolThread->threadParams = malloc(sizeof(spool_thread_params_t));
    newSpoolThread->threadParams->running = true;
    newSpoolThread->threadParams->smoothPool = spool;
    pthread_spin_lock(&spool->threadAccessLock);
    newSpoolThread->next = spool->threads;
    spool->threads = newSpoolThread;
    pthread_spin_unlock(&spool->threadAccessLock);
    pthread_create(&newSpoolThread->thread, NULL, spool_thread_run, newSpoolThread->threadParams);
    debug_print("Smoothing Pool\t- DISPATCHER   - Adding Worker Complete\n");
}

/* Sets the most recently started thread to the graveyard to die
        Always safe to use */
void spool_worker_remove(spool_t *spool) {
    debug_print("Smoothing Pool\t- Move Worker to Graveyard\n");
    pthread_spin_lock(&spool->threadAccessLock);
    spool_thread_t *oldThread = spool->threads;
    if (oldThread != NULL) {
        spool->threads = oldThread->next;
        oldThread->threadParams->running = false;
        oldThread->next = spool->threadGraveyard;
        spool->threadGraveyard = oldThread;
        debug_print("Smoothing Pool\t- DISPATCHER   - Move to Graveyard Successful\n");
    } else {
        debug_print("Smoothing Pool\t- DISPATCHER   - No Workers To Send To Graveyard.\n");
    }
    pthread_spin_unlock(&spool->threadAccessLock);
}

/* Initialises a new smoothing thread pool from the given job list
        jobList can be NULL */
spool_t *spool_init(spool_job_t *jobList) {
    spool_t *spool = malloc(sizeof(spool_t));
    debug_print("Smoothing Pool\t- DISPATCHER   - Initialise Begin\n");
    spool->threadGraveyard = NULL;
    spool->threads = NULL;

    pthread_spin_init(&spool->threadAccessLock, false);
    pthread_spin_init(&spool->jobQueueAccessLock, false);
    sem_init(&spool->jobWaitSem, false, 0);

    spool->nextJob = jobList;
    spool->lastJob = jobList;
    while (spool->lastJob != NULL && spool->lastJob->next != NULL) {
        spool->lastJob = spool->lastJob->next;
        sem_post(&spool->jobWaitSem);
        debug_print("Smoothing Pool\t- DISPATCHER   - Initialise Adding Job\n");
    }
    debug_print("Smoothing Pool\t- DISPATCHER   - Initialise Complete\n");
    return spool;
}

/* Stops all workers gracefully and returns any remaining jobs */
spool_job_t *spool_destroy(spool_t *spool) {
    debug_print("Smoothing Pool\t- DISPATCHER   - Destroy Begin\n");
    while (spool->threads != NULL) {
        spool_worker_remove(spool);
    }
    // Allow all graveyard threads to terminate (Only guarantees termination after there are no remaining live threads)
    for (spool_thread_t *threadPtr = spool->threadGraveyard; threadPtr != NULL; threadPtr = threadPtr->next) {
        debug_print("Smoothing Pool\t- DISPATCHER   - Flushing Graveyard (Submitted Dummy Job)\n");
        sem_post(&spool->jobWaitSem);
    }
    while (spool->threadGraveyard != NULL) {
        spool_thread_t *graveyardPop = spool->threadGraveyard;
        spool->threadGraveyard = graveyardPop->next;
        pthread_join(graveyardPop->thread, NULL);
        free(graveyardPop->threadParams);
        free(graveyardPop);
        debug_print("Smoothing Pool\t- DISPATCHER   - Removed Graveyard Worker\n");
    }
    sem_destroy(&spool->jobWaitSem);
    pthread_spin_destroy(&spool->jobQueueAccessLock);
    pthread_spin_destroy(&spool->threadAccessLock);
    spool_job_t *remainingJobs = spool->nextJob;
    free(spool);
    debug_print("Smoothing Pool\t- DISPATCHER   - Destroy Complete\n");
    return remainingJobs;
}


/* Waits until the remaining job count reaches 0 */
void spool_job_sync_wait(spool_job_sync_t *spoolJobSync) {
    pthread_mutex_lock(&spoolJobSync->countLock);
    while (spoolJobSync->count > 0) {
        debug_print("Smoothing Pool\t- DISPATCHER   - Waiting for jobs to finish. Remaining: %d\n", spoolJobSync->count);
        pthread_cond_wait(&spoolJobSync->endOfJob, &spoolJobSync->countLock);
    }
    debug_print("Smoothing Pool\t- DISPATCHER   - Batch Complete Detected\n");
    pthread_mutex_unlock(&spoolJobSync->countLock);
}

/* Tidy up the job sync. */
void spool_job_sync_destroy(spool_job_sync_t *spoolJobSync) {
    pthread_mutex_destroy(&spoolJobSync->countLock);
    pthread_cond_destroy(&spoolJobSync->endOfJob);
    free(spoolJobSync);
}