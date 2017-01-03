#include <stdio.h>
#include <stdlib.h>
#include "benchmarking/bmark.h"
#include "matrix/mat.h"
#include "debug.h"

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    debug_print("Node %d of %d - #%d\n", node + 1, totalNodes, node);

    if (argc == 4) {
        int size = atoi(argv[1]);
        double precision = strtof(argv[2], NULL);
        int type = atoi(argv[3]);
        switch (type) {
            case 1:
                bmark_mpi(size, precision);
                break;
            default:
                bmark_serial(size, precision);
        }
    } else {
        if(node == 0) {
            printf("For individual calculations use: %s <threads> <size> <precision> <type>\n", argv[0]);
            printf("  Ctr   |Ty|Size |MPI|  Acc   |  Time  |     Parity     |      CRC64     |\n");
        }
        double precision = 0.0001f;
        int size = 512;
        if(node == 0) {
            bmark_serial(size, precision);
        }
        // All other nodes must wait for node 0 to complete.
        MPI_Barrier(MPI_COMM_WORLD);
        bmark_mpi(size, precision);
    }

    MPI_Finalize();
    return 0;
}




