//
// Created by dominic on 22/12/16.
//

#include <stdbool.h>
#include "../matrix/mat.h"

#ifndef PARALLEL_COMPUTATION_CW2_DISPATCHER_H
#define PARALLEL_COMPUTATION_CW2_DISPATCHER_H

typedef struct DISPATCHER_TASK_T dispatcher_task_t;

dispatcher_task_t *dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit);

unsigned int dispatcher_task_loop_count(dispatcher_task_t *dispatcher_task);

mat_t *dispatcher_task_mat(dispatcher_task_t* dispatcher_task);

void dispatcher_mpi_share(dispatcher_task_t *dispatcher_task);

void dispatcher_task_run(dispatcher_task_t *dispatcher_task);

void dispatcher_task_destroy(dispatcher_task_t *dispatcher_task);

#endif //PARALLEL_COMPUTATION_CW2_DISPATCHER_H
