//
// Created by dominic on 31/10/16.
//

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "mat_itr.h"
#include "smoother.h"
#include "../debug.h"

typedef struct MAT_T {
    int xSize, ySize;
    double *data;
} mat_t;

/* Creates the matrix and allocates values */
mat_t *mat_init(double *values, int xSize, int ySize) {
    mat_t *matrix = malloc(sizeof(mat_t));
    matrix->data = values;
    matrix->xSize = xSize;
    matrix->ySize = ySize;
    return matrix;
}

/* Returns a matrix data pointer at the given coordinates
        Out of bounds coordinates will result in undefined behaviour */
double *mat_data_ptr(mat_t *matrix, long x, long y) {
    return matrix->data + x + (matrix->xSize * y);
}

/* Returns a matrix iterator starting at the given co-ordinates for the given width and height
        Out of bounds area will result in undefined behaviour */
mat_itr_t *mat_itr_create_partial(mat_t *matrix, long x, long y, long width, long height) {
    double *initialDataPtr = mat_data_ptr(matrix, x, y);
    return mat_itr_init(initialDataPtr, matrix->xSize, width, height);
}

mat_itr_edge_t *mat_itr_edge_create(mat_t *matrix) {
    return mat_itr_edge_init(matrix->data, matrix->xSize, matrix->ySize);
}

/* Initialises a matrix smoother for an area starting at the given co-ordinates for the given width and height
        Will smooth until doubles do not surpass limit.
        overLimit may be shared between multiple smoothers (thread safe).
        target Matrix *must* have same dimensions as source.
        */
smoother_t *smoother_create_partial(mat_t *source, mat_t *tmp,
                                      double limit, bool *overLimit,
                                      long x, long y,
                                      long xSize, long ySize,
                                      smoother_t *nextSmoother) {
    mat_itr_t *resItr = mat_itr_create_partial(tmp, x, y, xSize, ySize);
    mat_itr_t *ctrItr = mat_itr_create_partial(source, x, y, xSize, ySize);
    mat_itr_t *topItr = mat_itr_create_partial(source, x - 1, y, xSize, ySize);
    mat_itr_t *botItr = mat_itr_create_partial(source, x + 1, y, xSize, ySize);
    mat_itr_t *lftItr = mat_itr_create_partial(source, x, y - 1, xSize, ySize);
    mat_itr_t *rgtItr = mat_itr_create_partial(source, x, y + 1, xSize, ySize);
    return smoother_init(resItr, ctrItr, topItr, botItr, lftItr, rgtItr, overLimit, limit, nextSmoother);
}

/* Initialise a smoother for the inside of the matrix (excluding outer edge) */
smoother_t *smoother_single_init(mat_t *source, mat_t *target, double limit, bool *overLimit) {
    long itrWidth = target->xSize - 2;
    long itrHeight = target->ySize - 2;
    return smoother_create_partial(source, target, limit, overLimit, 1, 1, itrWidth, itrHeight, NULL);
}

/* Adds inner cut smooth jobs to head of give linked list */
smoother_t *smoother_multiple_init(mat_t *source,
                                     mat_t *tmp,
                                     double limit,
                                     bool *overLimit,
                                     unsigned int smthrSize) {
    long sections = (source->ySize - 2) / smthrSize;
    long remainderSection = (source->ySize - 2) % smthrSize;
    debug_print("Splitting %d matrix into %ld section/s of %d row/s with an extra %ld row section.\n",
                source->ySize, sections, smthrSize, remainderSection);
    long nextHeight = 1;
    smoother_t *currentSmtr = NULL;
    for (int i = 0; i < sections; i++) {
        debug_print("Section - [%ld:%d]\n", nextHeight, smthrSize);
        currentSmtr = smoother_create_partial(source, tmp,
                                               limit, overLimit,
                                               1, nextHeight,
                                               source->xSize - 2, smthrSize,
                                               currentSmtr);
        nextHeight += smthrSize;
    }
    if (remainderSection != 0) {
        debug_print("Section - [%ld:%ld]\n", nextHeight, remainderSection);
        currentSmtr = smoother_create_partial(source, tmp,
                                               limit, overLimit,
                                               1, nextHeight,
                                               source->xSize - 2, remainderSection,
                                               currentSmtr);
    }
    debug_print("Splitting Complete.\n");
    return currentSmtr;
}

/* Smooths the inside (exclude outer edge) of the given matrix until it changes < limit. */
mat_t *mat_smooth(mat_t *source, mat_t *target, double limit, bool *overLimit) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    do {
        *overLimit = false;
        smoother_t *smoother = smoother_single_init(source, target, limit, overLimit);
        smoother_run(smoother);
        smoother_destroy(smoother);

        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;

        ctr++;
    } while (*overLimit);

    printf("%08li,", ctr);

    return resultFlipped ? target : source;
}

/* Used for type punning between a double and a unsigned long long int */
union {
    double d; // C99 double == 64 bytes
    unsigned long long int ll; // C99 long long >= 64 bytes
} double_ulld_punner_u;

/* Calculates a 64 byte (C99) parity of the given matrix */
unsigned long long int mat_parity(mat_t *matrix) {
    unsigned long long int currentParity = 0ULL;
    double *endPtr = matrix->data + matrix->ySize * matrix->ySize;
    for (double *tmpPtr = matrix->data; tmpPtr < endPtr; tmpPtr++) {
        double_ulld_punner_u.d = *tmpPtr;
        currentParity ^= double_ulld_punner_u.ll;
    }
    return currentParity;
}

/* Calculates a 64 byte crc of the given Matrix */
unsigned long long int mat_crc64(mat_t *matrix) {
    unsigned long long int currentCRC = 0ULL;
    double *endPtr = matrix->data + matrix->ySize * matrix->ySize;
    for (double *tmpPtr = matrix->data; tmpPtr < endPtr; tmpPtr++) {
        double_ulld_punner_u.d = *tmpPtr;
        currentCRC += double_ulld_punner_u.ll;
    }
    return currentCRC;
}

/* Copies the outer edge from source to target matrix
    If matrices of different size, behaviour undefined */
void mat_copy_edge(mat_t *source, mat_t *target) {
    mat_itr_edge_t *edgeIterator1 = mat_itr_edge_create(source);
    mat_itr_edge_t *edgeIterator2 = mat_itr_edge_create(target);
    while (mat_itr_edge_hasNext(edgeIterator1)) {
        *mat_itr_edge_next(edgeIterator2) = *mat_itr_edge_next(edgeIterator1);
    }
    mat_itr_edge_destroy(edgeIterator1);
    mat_itr_edge_destroy(edgeIterator2);
}

/* Pretty prints the matrix to STDOUT */
void mat_print(mat_t *matrix) {
    long size = matrix->xSize * matrix->ySize;
    double *ptr = matrix->data;
    for (long i = 1; i <= size; i++) {
        if (i % matrix->xSize != 0) {
            printf("%f, ", *ptr++);
        } else {
            printf("%f\n", *ptr++);
        }
    }
}

/* Compares the two matrices for equality */
bool mat_equals(mat_t *matrix1, mat_t *matrix2) {
    if (matrix1->ySize == matrix2->ySize && matrix1->xSize == matrix2->xSize) {
        bool match = true;
        mat_itr_t *matIterator1 = mat_itr_create_partial(matrix1, 0, 0, matrix1->xSize, matrix1->ySize);
        mat_itr_t *matIterator2 = mat_itr_create_partial(matrix2, 0, 0, matrix2->xSize, matrix2->ySize);
        while (mat_itr_hasNext(matIterator1) && match) {
            match = *mat_itr_next(matIterator1) == *mat_itr_next(matIterator2);
        }
        mat_itr_destroy(matIterator1);
        mat_itr_destroy(matIterator2);
        return match;
    } else {
        return false;
    }
}

void mat_shareRows(mat_t* mat) {
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    if(node > 0 && node < totalNodes - 1) {
        MPI_Request request[2];
        MPI_Status status[2];
        double* top = mat_data_ptr(mat, 0, 1);
        double* bottom = mat_data_ptr(mat, 0, mat->ySize - 2);
        MPI_Isend(top, mat->xSize, MPI_DOUBLE, node-1, 1, MPI_COMM_WORLD, request);
        MPI_Isend(bottom, mat->xSize, MPI_DOUBLE, node+1, 2, MPI_COMM_WORLD, &request[1]);
        MPI_Waitall(2, request, status);
    } else if(node > 0) {
        double* top = mat_data_ptr(mat, 0, 1);
        MPI_Send(top, mat->xSize, MPI_DOUBLE, node-1, 1, MPI_COMM_WORLD);
    } else {
        double* bottom = mat_data_ptr(mat, 0, mat->ySize - 2);
        MPI_Send(bottom, mat->xSize, MPI_DOUBLE, node+1, 2, MPI_COMM_WORLD);
    }
}

void mat_acceptEdgeRows(mat_t* mat) {
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    if(node != 0) {
        double* top = mat_data_ptr(mat, 0, 0);
        MPI_Recv(top, mat->xSize, MPI_DOUBLE, node+1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if(node == totalNodes - 1) {
        double* bottom = mat_data_ptr(mat, 0, mat->ySize - 1);
        MPI_Recv(bottom, mat->xSize, MPI_DOUBLE, node-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

/* Frees the given matrix from memory. Matrix must not be used afterwards. */
void mat_destroy(mat_t *matrix) {
    free(matrix->data);
    free(matrix);
}


