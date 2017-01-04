//
// Created by dominic on 22/12/16.
//

#include <stdbool.h>
#include <stdlib.h>
#include "../matrix/mat.h"

typedef struct DISPATCHER_TASK_T {
    mat_t *srcMatrix;
    mat_t *tmpMatrix;
    smoother_t *src2tmp;
    smoother_t *tmp2src;
    smoother_t *nextJob;
    bool *active;
    unsigned int loopCtr;
} dispatcher_task_t;

dispatcher_task_t *dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit) {
    dispatcher_task_t *dispatcher = malloc(sizeof(dispatcher_task_t));
    dispatcher->srcMatrix = srcMatrix;
    dispatcher->tmpMatrix = tmpMatrix;
    dispatcher->active = overLimit;
    // Create the two possible jobs. One calculates from source to temp. The other from temp to source.
    dispatcher->src2tmp = smoother_single_init(srcMatrix, tmpMatrix, limit, dispatcher->active);
    dispatcher->tmp2src = smoother_single_init(tmpMatrix, srcMatrix, limit, dispatcher->active);
    // Keep a copy of the next job to run. These are single use and can be cloned in advance.
    dispatcher->nextJob = smoother_clone(dispatcher->src2tmp);
    dispatcher->loopCtr = 0;
    return dispatcher;
}

/* Returns the number of iterations the dispatcher has run. */
unsigned int dispatcher_task_loop_count(dispatcher_task_t *dispatcher_task) {
    return dispatcher_task->loopCtr;
}

/* Returns the most recently filled matrix */
mat_t *dispatcher_task_mat(dispatcher_task_t* dispatcher_task) {
    return dispatcher_task->loopCtr % 2 == 0 ? dispatcher_task->srcMatrix : dispatcher_task->tmpMatrix;
}

/* Shares the results of computation with other processes. Finds out if another iteration is required. */
void dispatcher_mpi_share(dispatcher_task_t *dispatcher_task) {
    // There will be a maximum of 5 transfers. Don't bother working out how many you actually need. This is free.
    MPI_Request mpiRequest[5];
    bool globallyOverLimit;
    mat_t *currentMat = dispatcher_task_mat(dispatcher_task);

    int transferCnt = 1; // There will be at least one reduce all.
    MPI_Iallreduce(dispatcher_task->active, &globallyOverLimit, 1,
                   MPI_C_BOOL, MPI_LOR,
                   MPI_COMM_WORLD, mpiRequest);
    // Kick off extra transfers, giving them a pointer to the important blank part of the transfer count.
    transferCnt += mat_mpi_shareRows(currentMat, mpiRequest + transferCnt);
    transferCnt += mat_mpi_acceptEdgeRows(currentMat, mpiRequest + transferCnt);

    // Prepare for the next execution. This can be done while waiting for the transfers to complete.
    smoother_destroy(dispatcher_task->nextJob);
    dispatcher_task->nextJob = smoother_clone(dispatcher_task->loopCtr % 2 == 0 ? dispatcher_task->src2tmp : dispatcher_task->tmp2src);

    // This will synchronise ALL processes because of the AllReduce
    MPI_Waitall(transferCnt, mpiRequest, MPI_STATUSES_IGNORE);
    *dispatcher_task->active = globallyOverLimit;
}

/* Run the dispatcher in a loop until the matrix doesn't need to be smoothed.
   Synchronised across all processes. Will deadlock if run individually! */
void dispatcher_task_run(dispatcher_task_t *dispatcher_task) {
    do {
        *dispatcher_task->active = false;
        smoother_run(dispatcher_task->nextJob);
        dispatcher_task->loopCtr++;
        dispatcher_mpi_share(dispatcher_task);
    } while (*dispatcher_task->active);
}

/* Safely destroy the dispatcher. */
void dispatcher_task_destroy(dispatcher_task_t *dispatcher_task) {
    smoother_destroy(dispatcher_task->src2tmp);
    smoother_destroy(dispatcher_task->tmp2src);
    smoother_destroy(dispatcher_task->nextJob);
    free(dispatcher_task);
}