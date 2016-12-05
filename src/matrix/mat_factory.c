//
// Created by dominic on 05/11/16.
//

#include <stdlib.h>
#include <string.h>
#include "mat.h"

const long RNG_SEED = 31413241L;

/* Creates a new matrix full of zero values */
mat_t *mat_factory_init_empty(long xSize, long ySize) {
    void *data = NULL;
    posix_memalign( &data, 64 , sizeof(double) * xSize * ySize);
    memset( data, 0, xSize * ySize * sizeof(double));
    return mat_init(data, xSize, ySize);
}

/* Creates a new matrix with seeded random values */
mat_t *mat_factory_init_random(long xSize, long ySize) {
    mat_t *matrix = mat_factory_init_empty(xSize, ySize);
    srand48(RNG_SEED);

    mat_itr_edge_t *matEdgeIterator = mat_itr_edge_create(matrix);
    while(mat_itr_edge_hasNext(matEdgeIterator)) {
        *mat_itr_edge_next(matEdgeIterator) = drand48();
    }
    mat_itr_edge_destroy(matEdgeIterator);

    return matrix;
}