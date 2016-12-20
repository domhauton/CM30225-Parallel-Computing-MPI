#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "matrix/bmark.h"
#include <mpi.h>
#include <stdbool.h>

void bootstrap(int argc, char *argv[]) {
    int tmp = 0;

    printf("Master sleeping.\n");
    sleep(3);
    printf("Master waiting.\n");
    MPI_Recv(&tmp, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    printf("Master received: %d\n", tmp);
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
        printf("For individual calculations use: %s <threads> <size> <precision> <type> <cut>\n", argv[0]);
        printf("  Ctr   |Ty|Size |Thr|Chnk|  Acc   |  Time  |     Parity     |      CRC64     |\n");
        double precision = 0.0001f;
        for (int matrixSize = (2 << 7); matrixSize <= (2 << 7); matrixSize <<= 1) {
            bmark_serial(matrixSize, precision);
            for (unsigned int threads = 1; threads <= (sysconf(_SC_NPROCESSORS_ONLN) * 2); threads <<= 1) {
                bmark_pool(matrixSize, precision, threads, 10);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);
    printf("Node %d in [0,%d)\n", node, totalNodes);
    bool isMaster = node == 0;

    if (isMaster) {
        printf("Starting Master Node.\n");
        bootstrap(argc, argv);
    } else {
        printf("Starting Slave Node.\n");
        int tmp = 0;
        MPI_Request mpi_request;
        MPI_Status mpi_status;
        printf("Slave - Sending\n");

        MPI_Isend(&tmp, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &mpi_request);
        printf("Slave - Sent\n");
        MPI_Wait(&mpi_request, &mpi_status);
        printf("Slave - Receive Confirmed\n");
    }
    MPI_Finalize();
    return 0;
}




