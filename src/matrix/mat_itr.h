//
// Created by dominic on 01/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_MATRIXSMOOTHER_H
#define PARALLEL_COMPUTATION_CW1_MATRIXSMOOTHER_H

#include <stdbool.h>

typedef struct MAT_ITR_T mat_itr_t;

typedef struct MAT_ITR_EDGE_T mat_itr_edge_t;

mat_itr_t *mat_itr_init(double *dataPtr, long fullWidth, long areaWidth, long areaHeight);

mat_itr_t *mat_itr_split(mat_itr_t *matIterator1);

double *mat_itr_next(mat_itr_t *matIterator);

double *mat_itr_step_unsafe(mat_itr_t *matIterator);

double *mat_itr_next_unsafe(mat_itr_t *matIterator);

bool mat_itr_hasNext(mat_itr_t *matIterator);

void mat_itr_destroy(mat_itr_t *matIterator);

mat_itr_edge_t *mat_itr_edge_init(double *data, long width, long height);

double *mat_itr_edge_next(mat_itr_edge_t *edgeIterator);

bool mat_itr_edge_hasNext(mat_itr_edge_t *edgeIterator);

void mat_itr_edge_destroy(mat_itr_edge_t *edgeIterator);

#endif //PARALLEL_COMPUTATION_CW1_MATRIXSMOOTHER_H
