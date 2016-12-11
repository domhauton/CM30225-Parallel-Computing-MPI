//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
#define PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H

#include "../matrix/mat.h"
#include "spool.h"

typedef struct DISPATCHER_TASK_T dispatcher_task_t;

dispatcher_task_t *dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize);

unsigned int dispatcher_task_loop_count(dispatcher_task_t *dispatcher_task);

mat_t *dispatcher_task_mat(dispatcher_task_t* dispatcher_task);

void dispatcher_task_run(dispatcher_task_t *dispatcher_task, spool_t *spool);

void dispatcher_task_destroy(dispatcher_task_t *dispatcher_task);

#endif //PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
