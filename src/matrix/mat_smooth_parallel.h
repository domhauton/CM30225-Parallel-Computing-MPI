//
// Created by dominic on 15/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_MATRIXPARALLEL_H
#define PARALLEL_COMPUTATION_CW1_MATRIXPARALLEL_H

#include "mat.h"

mat_t *mat_smooth_parallel_join(mat_t *source, mat_t *target, double limit, int threadCount);

mat_t *mat_smooth_parallel_barrier_leapfrog(mat_t *source, mat_t *target, double limit, unsigned int threadCount);

mat_t *mat_smooth_parallel_barrier_rowcut(mat_t *source,
                                          mat_t *target,
                                          double limit,
                                          unsigned int threadCount);

mat_t *mat_smooth_parallel_pool_rowcut(mat_t *source,
                                       mat_t *target,
                                       double limit,
                                       unsigned int threadCount,
                                       unsigned int smthrSize);

#endif //PARALLEL_COMPUTATION_CW1_MATRIXPARALLEL_H
