//
// Created by dominic on 05/11/16.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mat.h"

const long RNG_SEED = 31413241L;

/* Creates a new matrix full of zero values */
mat_t *mat_factory_init_empty(long xSize, long ySize) {
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
mat_t *mat_factory_init_seeded_skip(long xSize, long ySize, int skip) {
    mat_t *matrix = mat_factory_init_empty(xSize, ySize);
    srand48(RNG_SEED);
    //Skip all seeded before
    for(int i = 0; i < xSize * skip; i++) {
        drand48();
    }

    mat_itr_t *matItr = mat_itr_create_partial(matrix, 0, 0, xSize, ySize);
    while(mat_itr_hasNext(matItr)) {
        *mat_itr_next(matItr) = drand48();
    }
    mat_itr_destroy(matItr);

    return matrix;
}

/* Creates a new matrix with seeded random values */
mat_t *mat_factory_init_seeded(int xSize, int ySize) {
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    int globalWorkingSize = ySize-2;
    int localWorkingSize = globalWorkingSize/totalNodes;
    int remainder = globalWorkingSize%totalNodes;
    if(remainder > node) {
        localWorkingSize++;
    }
    int extraRows = remainder > node ? node : remainder;
    int skipRows = (localWorkingSize*node) + extraRows;
    printf("Node %d - Extra rows: %d\n", node, extraRows);
    int localYSize = localWorkingSize + 2;
    printf("Node %d work start %d for %d rows. Storing start %d for %d rows\n", node, skipRows+1, localWorkingSize, skipRows, localYSize);
    return mat_factory_init_seeded_skip(xSize, localYSize, skipRows);
}
