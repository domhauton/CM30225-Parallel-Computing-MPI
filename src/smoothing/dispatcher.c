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
    dispatcher->src2tmp = smoother_single_init(srcMatrix, tmpMatrix, limit, dispatcher->active);
    dispatcher->tmp2src = smoother_single_init(tmpMatrix, srcMatrix, limit, dispatcher->active);
    dispatcher->nextJob = smoother_clone(dispatcher->src2tmp);
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

    smoother_destroy(dispatcher_task->nextJob);
    dispatcher_task->nextJob = smoother_clone(dispatcher_task->loopCtr % 2 == 0 ? dispatcher_task->src2tmp : dispatcher_task->tmp2src);

    MPI_Waitall(cnt, mpiRequest, MPI_STATUSES_IGNORE);
    *dispatcher_task->active = globallyOverLimit;
}

void dispatcher_task_run(dispatcher_task_t *dispatcher_task) {
    do {
        *dispatcher_task->active = false;

        smoother_run(dispatcher_task->nextJob);

        dispatcher_task->loopCtr++;
        dispatcher_mpi_share(dispatcher_task);
    } while (*dispatcher_task->active);
}


void dispatcher_task_destroy(dispatcher_task_t *dispatcher_task) {
    smoother_destroy(dispatcher_task->src2tmp);
    smoother_destroy(dispatcher_task->tmp2src);
    smoother_destroy(dispatcher_task->nextJob);
    free(dispatcher_task);
}