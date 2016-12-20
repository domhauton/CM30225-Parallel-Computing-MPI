#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "matrix/bmark.h"
#include <mpi.h>
#include <stdbool.h>

#define MATRIX_CUT_SIZE 10

void bootstrap(int argc, char *argv[]) {
    if (argc == 6) {
        unsigned int threads = (unsigned int) atoi(argv[1]);
        int size = atoi(argv[2]);
        double precision = strtof(argv[3], NULL);
        int type = atoi(argv[4]);
        unsigned int cut = (unsigned int) atoi(argv[5]);
        switch (type) {
            case 1:
                bmark_pool(size, precision, threads, cut);
                break;
            case 0:
            default:
                bmark_serial(size, precision);
        }
    } else {
        //printf("For individual calculations use: %s <threads> <size> <precision> <type> <cut>\n", argv[0]);
        //printf("  Ctr   |Ty|Size |Thr|Chnk|  Acc   |  Time  |     Parity     |      CRC64     |\n");
        double precision = 0.0001f;
        //for (int matrixSize = (2 << 4); matrixSize <= (2 << 4); matrixSize <<= 1) {
            //bmark_serial(8, precision);
            bmark_pool(8, precision, (unsigned int) sysconf(_SC_NPROCESSORS_ONLN) + 2, MATRIX_CUT_SIZE);
        //}
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    printf("Node %d of %d - #%d\n", node + 1, totalNodes, node);
    bootstrap(argc, argv);
    MPI_Finalize();
    return 0;
}




