//
// Created by dominic on 05/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H
#define PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H

#include "mat_itr.h"

typedef struct smoother_T smoother_t;

smoother_t *smoother_init(mat_itr_t *target,
                            mat_itr_t *srcCenter,
                            mat_itr_t *srcUp,
                            mat_itr_t *srcDown,
                            mat_itr_t *srcLeft,
                            mat_itr_t *srcRight,
                            bool *overLimit,
                            double diffLimit,
                            smoother_t *next);

smoother_t *smoother_clone(smoother_t* smoother_old);

smoother_t *smoother_next(smoother_t *matSmoother);

unsigned int smoother_child_jobs(smoother_t *matSmoother);

void smoother_run(smoother_t *matSmoother);

void smoother_destroy(smoother_t *matSmoother);

void smoother_destroy_chain(smoother_t *smoother);

#endif //PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H
