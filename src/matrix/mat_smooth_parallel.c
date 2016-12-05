//
// Created by dominic on 15/11/16.
//

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "mat_smooth_parallel.h"
#include "spool.h"

void *parallelSmoothHelper(void *param) {
    mat_smthr_smooth(param);
    return NULL;
}

/* Parallel wrapper for mat_smooth
        Implemented using joins as synchronization method */
mat_t *mat_smooth_parallel_join(mat_t *source, mat_t *target, double limit, int threadCount) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    pthread_t threads[threadCount];
    mat_smthr_t *smoothers[threadCount];
    bool *overLimit = malloc(sizeof(int));
    do {
        *overLimit = false;
        smoothers[0] = mat_smthr_create_inner(source, target, limit, overLimit);
        for (int pos = 2; pos <= threadCount; pos++) {
            smoothers[pos - 1] = mat_smthr_split(smoothers[(pos - (pos >> 1)) - 1]);
        }
        for (int i = 0; i < threadCount; i++) {
            pthread_create(&threads[i], NULL, parallelSmoothHelper, smoothers[i]);
        }
        //long totItrCnt = 0;
        for (int i = 0; i < threadCount; i++) {
            if (pthread_join(threads[i], NULL)) {
                printf("Error joining threads.\n");
            }
            //totItrCnt += MatSmoother_getIterations(smoothers[i]);
            mat_smthr_destroy(smoothers[i]);
        }
        //printf("Smooth Ops (sqrt): %f\n", sqrt(totItrCnt));
        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;
        ctr++;
    } while (*overLimit);

    free(overLimit);
    printf("%08li,", ctr);

    return resultFlipped ? source : target;
}

struct parallelArguments {
    mat_smthr_t *matSmoother;
    pthread_barrier_t *startWorkBarrier;
    pthread_barrier_t *endWorkBarrier;
    bool alive;
};

void *parallelMutexSmoothHelper(void *param) {
    struct parallelArguments *arguments = param;
    pthread_barrier_wait(arguments->startWorkBarrier);
    while (arguments->alive) {
        if (arguments->matSmoother != NULL) {
            mat_smthr_smooth(arguments->matSmoother);
        }
        pthread_barrier_wait(arguments->endWorkBarrier);
        pthread_barrier_wait(arguments->startWorkBarrier);
    }
    return NULL;
}

/* Parallel wrapper for mat_smooth
        Implemented using barriers as synchronization method and leapfrog labour division */
mat_t *mat_smooth_parallel_barrier_leapfrog(mat_t *source, mat_t *target, double limit, unsigned int threadCount) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    pthread_t threads[threadCount];
    pthread_barrier_t *startWorkBarrier = malloc(sizeof(pthread_barrier_t));
    pthread_barrier_t *endWorkBarrier = malloc(sizeof(pthread_barrier_t));
    struct parallelArguments pArgs[threadCount];

    pthread_barrier_init(startWorkBarrier, NULL, threadCount + 1);

    for (int i = 0; i < threadCount; i++) {
        pArgs[i].startWorkBarrier = startWorkBarrier;
        pArgs[i].endWorkBarrier = endWorkBarrier;
        pArgs[i].alive = true;
        pthread_create(&threads[i], NULL, parallelMutexSmoothHelper, &pArgs[i]);
    }

    bool *overLimit = malloc(sizeof(int));

    do {
        *overLimit = false;
        // Create Jobs
        pArgs[0].matSmoother = mat_smthr_create_inner(source, target, limit, overLimit);
        for (int pos = 2; pos <= threadCount; pos++) {
            pArgs[pos - 1].matSmoother = mat_smthr_split(pArgs[(pos - (pos >> 1)) - 1].matSmoother);
        }

        pthread_barrier_init(endWorkBarrier, NULL, threadCount + 1);

        // Start work & reset
        pthread_barrier_wait(startWorkBarrier);
        pthread_barrier_destroy(startWorkBarrier);
        pthread_barrier_init(startWorkBarrier, NULL, threadCount + 1);

        // End work & reset
        pthread_barrier_wait(endWorkBarrier);
        pthread_barrier_destroy(endWorkBarrier);

        // Reduce
        for (int i = 0; i < threadCount; i++) {
            mat_smthr_destroy(pArgs[i].matSmoother);
        }

        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;
        ctr++;
    } while (*overLimit);

    // Tidy and join
    for (int i = 0; i < threadCount; i++) {
        pArgs[i].alive = false;
    }

    pthread_barrier_wait(startWorkBarrier);
    pthread_barrier_destroy(startWorkBarrier);

    free(startWorkBarrier);
    free(endWorkBarrier);
    free(overLimit);

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("%08li,", ctr);

    return resultFlipped ? source : target;
}

/* Parallel wrapper for mat_smooth
        Implemented using barriers as synchronization method and rowcut labour division */
mat_t *mat_smooth_parallel_barrier_rowcut(mat_t *source,
                                          mat_t *target,
                                          double limit,
                                          unsigned int threadCount) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    pthread_t threads[threadCount];
    pthread_barrier_t *startWorkBarrier = malloc(sizeof(pthread_barrier_t));
    pthread_barrier_t *endWorkBarrier = malloc(sizeof(pthread_barrier_t));
    bool *overLimit = malloc(sizeof(int));
    struct parallelArguments pArgs[threadCount];

    mat_smthr_t *smootherPtrs[threadCount];

    pthread_barrier_init(endWorkBarrier, NULL, threadCount + 1);
    pthread_barrier_init(startWorkBarrier, NULL, threadCount + 1);

    for (int i = 0; i < threadCount; i++) {
        pArgs[i].startWorkBarrier = startWorkBarrier;
        pArgs[i].endWorkBarrier = endWorkBarrier;
        pArgs[i].alive = true;
        pthread_create(&threads[i], NULL, parallelMutexSmoothHelper, &pArgs[i]);
    }

    do {
        *overLimit = false;
        // Create Jobs
        mat_smthr_create_inner_rowcut(source, target, limit, overLimit, threadCount, smootherPtrs);
        for (int i = 0; i < threadCount; i++) {
            pArgs[i].matSmoother = smootherPtrs[i];
        }

        pthread_barrier_init(endWorkBarrier, NULL, threadCount + 1);

        // Start work & reset
        pthread_barrier_wait(startWorkBarrier);
        pthread_barrier_destroy(startWorkBarrier);
        pthread_barrier_init(startWorkBarrier, NULL, threadCount + 1);

        // End work & reset
        pthread_barrier_wait(endWorkBarrier);
        pthread_barrier_destroy(endWorkBarrier);

        // Reduce
        //printf("[");
        for (int i = 0; i < threadCount; i++) {
            mat_smthr_destroy(pArgs[i].matSmoother);
            //printf("%f, ", pArgs[i].workTime);
        }
        //printf("]\n");
        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;
        ctr++;
    } while (*overLimit);

    // Tidy and join
    for (int i = 0; i < threadCount; i++) {
        pArgs[i].alive = false;
    }

    pthread_barrier_wait(startWorkBarrier);
    pthread_barrier_destroy(startWorkBarrier);
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    free(startWorkBarrier);
    free(endWorkBarrier);
    free(overLimit);

    printf("%08li,", ctr);

    return resultFlipped ? source : target;
}

/* Parallel wrapper for mat_smooth
        Implemented using worker pool synchronization method and rowcut labour division */
mat_t *mat_smooth_parallel_pool_rowcut(mat_t *source,
                                          mat_t *target,
                                          double limit,
                                          unsigned int threadCount,
                                          unsigned int smthrSize) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    bool *overLimit = malloc(sizeof(int));
    spool_t *spool = spool_init(NULL);

    for(int i = 0; i < threadCount; i++) {
        spool_worker_add(spool);
    }

    mat_smthr_list_t *jobRoot = malloc(sizeof(mat_smthr_list_t));
    jobRoot->data = NULL;

    do {
        *overLimit = false;
        // Submit Jobs
        long jobCount = mat_smthr_create_inner_cut_even(source, target, limit, overLimit, smthrSize, jobRoot);
        spool_job_sync_t *spoolJobSync = spool_job_add_batch(spool, jobRoot->next, (unsigned int) jobCount);
        spool_job_sync_wait(spoolJobSync);
        spool_job_sync_destroy(spoolJobSync);
        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;
        ctr++;
    } while (*overLimit);

    spool_destroy(spool);
    free(jobRoot);
    free(overLimit);

    printf("%08li,", ctr);

    return resultFlipped ? source : target;
}

