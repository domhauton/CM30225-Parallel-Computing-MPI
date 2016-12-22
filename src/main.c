#include <stdio.h>
#include <stdlib.h>
#include "benchmarking/bmark.h"
#include "matrix/mat.h"

#define MATRIX_CUT_SIZE 1000

void bootstrap(int argc, char *argv[], int node) {
    if (argc == 6) {
        unsigned int threads = (unsigned int) atoi(argv[1]);
        int size = atoi(argv[2]);
        double precision = strtof(argv[3], NULL);
        int type = atoi(argv[4]);
        unsigned int cut = (unsigned int) atoi(argv[5]);
        switch (type) {
            case 1:
                bmark_mpi_pool(size, precision, threads, cut);
                break;
            case 0:
            default:
                bmark_serial(size, precision);
        }
    } else {
        //printf("For individual calculations use: %s <threads> <size> <precision> <type> <cut>\n", argv[0]);

        double precision = 0.0001f;
        int size = 256;
        if(node == 0) {
            bmark_serial(size, precision);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        bmark_mpi(size, precision);
        MPI_Barrier(MPI_COMM_WORLD);
        bmark_mpi_pool(size, precision, 1, MATRIX_CUT_SIZE);
//        bmark_mpi_pool(size, precision, (unsigned int) sysconf(_SC_NPROCESSORS_ONLN) + 2, MATRIX_CUT_SIZE);
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    printf("Node %d of %d - #%d\n", node + 1, totalNodes, node);
    MPI_Barrier(MPI_COMM_WORLD);
    if(node == 0) {
        printf("  Ctr   |Ty|Size |Thr|  Acc   |  Time  |     Parity     |      CRC64     |\n");
    }
    bootstrap(argc, argv, node);
    MPI_Finalize();
    return 0;
}




