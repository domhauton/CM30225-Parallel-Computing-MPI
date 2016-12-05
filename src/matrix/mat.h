//
// Created by dominic on 31/10/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H
#define PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H

#include "mat_itr.h"
#include "mat_smthr.h"

typedef struct MAT_T mat_t;

mat_t *mat_init(double *values, long xSize, long ySize);

double *mat_data_ptr(mat_t *matrix, long x, long y);

mat_itr_t *mat_itr_create_partial(mat_t *matrix, long x, long y, long width, long height);

mat_itr_edge_t *mat_itr_edge_create(mat_t *matrix);

void mat_copy_edge(mat_t *source, mat_t *target);

mat_smthr_t *mat_smthr_create_inner(mat_t *source, mat_t *target, double limit, bool *overLimit);

void mat_smthr_create_inner_rowcut(mat_t *source, mat_t *tmp,
                                   double limit, bool *overLimit,
                                   unsigned int sections, mat_smthr_t **smoothers);

long mat_smthr_create_inner_cut_even(mat_t *source, mat_t *tmp,
                                     double limit, bool *overLimit,
                                     unsigned int smthrSize, mat_smthr_list_t *matSmthrLinkedList);

mat_t *mat_smooth(mat_t *source, mat_t *target, double limit, bool *overLimit);

void mat_print(mat_t *matrix);

bool mat_equals(mat_t *matrix1, mat_t *matrix2);

unsigned long long int mat_parity(mat_t *matrix);

unsigned long long int mat_crc64(mat_t *matrix);

void mat_destroy(mat_t *matrix);

#endif //PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H
