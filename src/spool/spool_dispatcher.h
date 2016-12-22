//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
#define PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H

#include "../matrix/mat.h"
#include "spool.h"

typedef struct spool_dispatcher_TASK_T spool_dispatcher_task_t;

spool_dispatcher_task_t *spool_dispatcher_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize);

unsigned int spool_dispatcher_task_loop_count(spool_dispatcher_task_t *spool_dispatcher_task);

mat_t *spool_dispatcher_task_mat(spool_dispatcher_task_t* spool_dispatcher_task);

void spool_dispatcher_task_run(spool_dispatcher_task_t *spool_dispatcher_task, spool_t *spool);

void spool_dispatcher_task_destroy(spool_dispatcher_task_t *spool_dispatcher_task);

#endif //PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
