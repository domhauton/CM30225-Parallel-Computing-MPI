//
// Created by dominic on 08/12/16.
//

#include <stdlib.h>
#include "dispatcher.h"

typedef struct DISPATCHER_TASK_T {
    mat_t *srcMatrix;
    mat_t *tmpMatrix;
    smoother_t *src2tmp;
    smoother_t *tmp2src;
    bool *active;
    unsigned int loopCtr;
} dispatcher_task_t;

dispatcher_task_t *dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize) {
    dispatcher_task_t *dispatcher = malloc(sizeof(dispatcher_task_t));
    dispatcher->srcMatrix = srcMatrix;
    dispatcher->tmpMatrix = tmpMatrix;
    dispatcher->active = overLimit;
    dispatcher->src2tmp = smoother_multiple_init(srcMatrix, tmpMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->tmp2src = smoother_multiple_init(tmpMatrix, srcMatrix, limit, dispatcher->active, smthrSize);
    dispatcher->loopCtr = 0;
    return dispatcher;
}

unsigned int dispatcher_task_loop_count(dispatcher_task_t *dispatcher_task) {
    return dispatcher_task->loopCtr;
}

mat_t *dispatcher_task_mat(dispatcher_task_t* dispatcher_task) {
    return dispatcher_task->loopCtr % 2 == 0 ? dispatcher_task->srcMatrix : dispatcher_task->tmpMatrix;
}

void dispatcher_mpi_share(dispatcher_task_t *dispatcher_task) {
    MPI_Request mpiRequest[5];
    bool globallyOverLimit;
    mat_t *currentMat = dispatcher_task_mat(dispatcher_task);

    int cnt = 0;
    MPI_Iallreduce(dispatcher_task->active, &globallyOverLimit, 1,
                   MPI_C_BOOL, MPI_LOR,
                   MPI_COMM_WORLD, mpiRequest);
    cnt++;
    cnt += mat_shareRows(currentMat, mpiRequest+cnt);
    cnt += mat_acceptEdgeRows(currentMat, mpiRequest+cnt);

    MPI_Waitall(cnt, mpiRequest, MPI_STATUSES_IGNORE);
    *dispatcher_task->active = globallyOverLimit;
}

void dispatcher_task_run(dispatcher_task_t *dispatcher_task, spool_t *spool) {
    do {
        *dispatcher_task->active = false;
        smoother_t* jobList = smoother_clone(dispatcher_task->loopCtr % 2 == 0 ? dispatcher_task->src2tmp : dispatcher_task->tmp2src);
        // Submit Jobs
        spool_job_sync_t *spoolJobSync = spool_job_add_batch(spool, jobList);
        spool_job_sync_wait(spoolJobSync);
        spool_job_sync_destroy(spoolJobSync);

        dispatcher_task->loopCtr++;
        dispatcher_mpi_share(dispatcher_task);
    } while (*dispatcher_task->active);
}


void dispatcher_task_destroy(dispatcher_task_t *dispatcher_task) {
    smoother_destroy_chain(dispatcher_task->src2tmp);
    smoother_destroy_chain(dispatcher_task->tmp2src);
    free(dispatcher_task);
}