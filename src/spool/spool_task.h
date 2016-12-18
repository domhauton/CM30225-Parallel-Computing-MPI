//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
#define PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H

#include "../matrix/mat.h"
#include "spool.h"

typedef struct SPOOL_TASK_T spool_task_t;

spool_task_t *spool_task_init(mat_t *srcMatrix,
                                        mat_t *tmpMatrix,
                                        double limit,
                                        bool *overLimit,
                                        unsigned int smthrSize,
                                        unsigned int expectedMatUpdates);

unsigned int spool_task_loop_count(spool_task_t *spool_task);

mat_t *spool_task_mat(spool_task_t* spool_task);

void spool_task_run(spool_task_t *spool_task, spool_t *spool);

void spool_task_destroy(spool_task_t *spool_task);

#endif //PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
