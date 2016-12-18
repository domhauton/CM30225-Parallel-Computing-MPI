//
// Created by dominic on 08/12/16.
//

#include <stdlib.h>
#include "spool_task.h"

typedef struct SPOOL_TASK_T {
    mat_t *srcMatrix;
    mat_t *tmpMatrix;
    smoother_t *src2tmp;
    smoother_t *tmp2src;
    bool *active;
    unsigned int loopCtr;
    // External information
    unsigned int expectedMatUpdates;
    sem_t *matUpdateWaitSem;
} spool_task_t;

spool_task_t *spool_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize,
                                        unsigned int expectedMatUpdates) {
    spool_task_t *dispatcher = malloc(sizeof(spool_task_t));
    dispatcher->srcMatrix = srcMatrix;
    dispatcher->tmpMatrix = tmpMatrix;
    dispatcher->active = overLimit;
    dispatcher->src2tmp = smoother_multiple_init(srcMatrix, tmpMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->tmp2src = smoother_multiple_init(tmpMatrix, srcMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->loopCtr = 0;
    if(expectedMatUpdates == NULL || expectedMatUpdates <= 0)  {
        dispatcher->expectedMatUpdates = expectedMatUpdates;
        dispatcher->matUpdateWaitSem = malloc(sizeof(sem_t));
    }
    return dispatcher;
}

unsigned int spool_task_loop_count(spool_task_t *spool_task) {
    return spool_task->loopCtr;
}

mat_t *spool_task_mat(spool_task_t* spool_task) {
    return spool_task->loopCtr % 2 == 0 ? spool_task->srcMatrix : spool_task->tmpMatrix;
}

void initMatUpdateSem(spool_task_t *spool_task) {
    if(spool_task->expectedMatUpdates != NULL) {
        sem_init(spool_task->matUpdateWaitSem, false, spool_task->expectedMatUpdates);
    }
}

void waitMatUpdateAndRst(spool_task_t *spool_task) {
    if(spool_task->expectedMatUpdates != NULL) {
        sem_wait(spool_task->matUpdateWaitSem);
        sem_destroy(spool_task->matUpdateWaitSem);
        initMatUpdateSem(spool_task);
    }
}

void spool_task_run(spool_task_t *spool_task, spool_t *spool) {
    initMatUpdateSem(spool_task);
    do {
        *spool_task->active = false;

        // Wait for matrix to have outer rows inserted
        waitMatUpdateAndRst(spool_task);

        smoother_t* jobList = smoother_clone(spool_task->loopCtr % 2 == 0 ? spool_task->src2tmp : spool_task->tmp2src);
        // Submit Jobs
        spool_job_sync_t *spoolJobSync = spool_job_add_batch(spool, jobList);
        spool_job_sync_wait(spoolJobSync);
        spool_job_sync_destroy(spoolJobSync);
        spool_task->loopCtr++;
    } while (*spool_task->active);
}

void spool_task_destroy(spool_task_t *spool_task) {
    smoother_destroy_chain(spool_task->src2tmp);
    smoother_destroy_chain(spool_task->tmp2src);
    waitMatUpdateAndRst(spool_task);
    free(spool_task->matUpdateWaitSem);
    free(spool_task);
}