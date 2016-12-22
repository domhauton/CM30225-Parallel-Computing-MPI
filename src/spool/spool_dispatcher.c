//
// Created by dominic on 08/12/16.
//

#include <stdlib.h>
#include "spool_dispatcher.h"

typedef struct spool_dispatcher_TASK_T {
    mat_t *srcMatrix;
    mat_t *tmpMatrix;
    smoother_t *src2tmp;
    smoother_t *tmp2src;
    bool *active;
    unsigned int loopCtr;
} spool_dispatcher_task_t;

spool_dispatcher_task_t *spool_dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize) {
    spool_dispatcher_task_t *dispatcher = malloc(sizeof(spool_dispatcher_task_t));
    dispatcher->srcMatrix = srcMatrix;
    dispatcher->tmpMatrix = tmpMatrix;
    dispatcher->active = overLimit;
    dispatcher->src2tmp = smoother_multiple_init(srcMatrix, tmpMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->tmp2src = smoother_multiple_init(tmpMatrix, srcMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->loopCtr = 0;
    return dispatcher;
}

unsigned int spool_dispatcher_task_loop_count(spool_dispatcher_task_t *spool_dispatcher_task) {
    return spool_dispatcher_task->loopCtr;
}

mat_t *spool_dispatcher_task_mat(spool_dispatcher_task_t* spool_dispatcher_task) {
    return spool_dispatcher_task->loopCtr % 2 == 0 ? spool_dispatcher_task->srcMatrix : spool_dispatcher_task->tmpMatrix;
}

void spool_dispatcher_mpi_share(spool_dispatcher_task_t *spool_dispatcher_task) {
    MPI_Request mpiRequest[5];
    bool globallyOverLimit;
    mat_t *currentMat = spool_dispatcher_task_mat(spool_dispatcher_task);

    int cnt = 0;
    MPI_Iallreduce(spool_dispatcher_task->active, &globallyOverLimit, 1,
                   MPI_C_BOOL, MPI_LOR,
                   MPI_COMM_WORLD, mpiRequest);
    cnt++;
    cnt += mat_shareRows(currentMat, mpiRequest+cnt);
    cnt += mat_acceptEdgeRows(currentMat, mpiRequest+cnt);

    MPI_Waitall(cnt, mpiRequest, MPI_STATUSES_IGNORE);
    *spool_dispatcher_task->active = globallyOverLimit;
}

void spool_dispatcher_task_run(spool_dispatcher_task_t *spool_dispatcher_task, spool_t *spool) {
    do {
        *spool_dispatcher_task->active = false;
        smoother_t* jobList = smoother_clone(spool_dispatcher_task->loopCtr % 2 == 0 ? spool_dispatcher_task->src2tmp : spool_dispatcher_task->tmp2src);
        // Submit Jobs
        spool_job_sync_t *spoolJobSync = spool_job_add_batch(spool, jobList);
        spool_job_sync_wait(spoolJobSync);
        spool_job_sync_destroy(spoolJobSync);

        spool_dispatcher_task->loopCtr++;
        spool_dispatcher_mpi_share(spool_dispatcher_task);
    } while (*spool_dispatcher_task->active);
}


void spool_dispatcher_task_destroy(spool_dispatcher_task_t *spool_dispatcher_task) {
    smoother_destroy(spool_dispatcher_task->src2tmp);
    smoother_destroy(spool_dispatcher_task->tmp2src);
    free(spool_dispatcher_task);
}