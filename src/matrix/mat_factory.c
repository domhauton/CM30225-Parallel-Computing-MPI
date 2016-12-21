//
// Created by dominic on 05/11/16.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mat.h"

#define ROOT_RANK 0
const long RNG_SEED = 31413241L;

/* Creates a new matrix full of zero values */
mat_t *mat_factory_init_empty(int xSize, int ySize) {
    void *data = NULL;
    int success = posix_memalign( &data, 64 , sizeof(double) * xSize * ySize) == 0;
    if(success) {
        memset( data, 0, xSize * ySize * sizeof(double));
        return mat_init(data, xSize, ySize);
    } else {
        perror("Failed to allocate required aligned memory. Exiting");
        exit(1);
    }


}

/* Creates a new matrix with seeded random values */
mat_t *mat_factory_init_seeded(int xSize, int ySize) {
    mat_t *matrix = mat_factory_init_empty(xSize, ySize);
    srand48(RNG_SEED);

    mat_itr_edge_t *matEdgeIterator = mat_itr_edge_create(matrix);
    while(mat_itr_edge_hasNext(matEdgeIterator)) {
        *mat_itr_edge_next(matEdgeIterator) = drand48();
    }
    mat_itr_edge_destroy(matEdgeIterator);

    return matrix;
}

void calc_node_compute_size(int *count, int *displacement, int nodeCount, int computeHeight, int rowWidth) {
    //printf("%d/%d=%d\n", computeHeight, nodeCount, computeHeight/nodeCount);
    //printf("%dmod%d=%d\n", computeHeight, nodeCount, computeHeight%nodeCount);
    int minJobSize = computeHeight/nodeCount;
    int processesWithExtra = computeHeight%nodeCount;
    int currentDisplacement = 0;
    for(int i = 0; i < nodeCount; i++) {
        int jobSize = (minJobSize + (i<processesWithExtra ? 1 : 0)) * rowWidth;
        *displacement++ = currentDisplacement;
        *count++ = jobSize + (2 * rowWidth);
        currentDisplacement += jobSize;
    }
}

/* Creates a new matrix with seeded random values */
mat_t *mat_scatter(mat_t *mat, int xSize, int ySize) {
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    int displacement[totalNodes];
    int count[totalNodes];
    calc_node_compute_size(count, displacement, totalNodes, ySize-2, xSize);
    mat_t *localMat = mat_factory_init_empty(xSize, count[node]/xSize);
    MPI_Scatterv(
            mat_data_ptr(mat, 0, 0), count, displacement, MPI_DOUBLE,
            mat_data_ptr(localMat, 0, 0), count[node], MPI_DOUBLE,
            ROOT_RANK, MPI_COMM_WORLD);
    return localMat;
}
