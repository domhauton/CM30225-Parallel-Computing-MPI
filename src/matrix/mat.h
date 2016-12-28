//
// Created by dominic on 31/10/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H
#define PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H

#include <mpi.h>
#include "mat_itr.h"
#include "../smoothing/smoother.h"

typedef struct MAT_T mat_t;

mat_t *mat_init(double *values, int xSize, int ySize);

mat_t *mat_init_clone_edge(mat_t *matrix);

double *mat_data_ptr(mat_t *matrix, long x, long y);

mat_itr_t *mat_itr_create_partial(mat_t *matrix, long x, long y, long width, long height);

mat_itr_edge_t *mat_itr_edge_create(mat_t *matrix);

void mat_copy_edge(mat_t *source, mat_t *target);

smoother_t *smoother_single_init(mat_t *source, mat_t *target,
                                   double limit, bool *overLimit);

mat_t *mat_smooth(mat_t *source, mat_t *target, double limit, bool *overLimit);

void mat_print_local(mat_t *matrix);

void mat_print_mpi(mat_t *matrix);

bool mat_equals(mat_t *matrix1, mat_t *matrix2);

unsigned long long int mat_parity(mat_t *matrix);

unsigned long long int mat_crc64(mat_t *matrix);

unsigned long long int mat_parity_local(mat_t *matrix);

unsigned long long int mat_crc64_local(mat_t *matrix);

int mat_shareRows(mat_t* mat, MPI_Request* mpi_request);

int mat_acceptEdgeRows(mat_t* mat, MPI_Request* mpi_request);

void mat_destroy(mat_t *matrix);

#endif //PARALLEL_COMPUTATION_CW1_RELAXATIONMATRIX_H
