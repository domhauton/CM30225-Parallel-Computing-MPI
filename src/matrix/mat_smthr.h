//
// Created by dominic on 05/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H
#define PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H

#include "mat_itr.h"

typedef struct MAT_SMTHR_T mat_smthr_t;

typedef struct MAT_SMTHR_LIST_T{
    mat_smthr_t *data;
    struct MAT_SMTHR_LIST_T *next;
} mat_smthr_list_t;

mat_smthr_t *mat_smthr_init(mat_itr_t *target,
                            mat_itr_t *srcCenter,
                            mat_itr_t *srcUp,
                            mat_itr_t *srcDown,
                            mat_itr_t *srcLeft,
                            mat_itr_t *srcRight,
                            bool *overLimit,
                            double diffLimit);

void mat_smthr_smooth(mat_smthr_t *matSmoother);

mat_smthr_t *mat_smthr_split(mat_smthr_t *matSmoother1);

void mat_smthr_destroy(mat_smthr_t *matSmoother);

#endif //PARALLEL_COMPUTATION_CW1_MATSMOOTHER_H
