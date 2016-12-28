//
// Created by dominic on 16/11/16.
//

#include <bits/time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "../matrix/mat.h"
#include "../matrix/mat_factory.h"
#include "bmark.h"
#include "../smoothing/dispatcher.h"

double bmark_serial(int size, double precision) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_seeded(size, size);
    bool overLimit;
    gettimeofday(&tv_start, NULL);
    mat_t *tmp = mat_init_clone_edge(matrix1);
    mat_t *retMat = mat_smooth(matrix1, tmp, precision, &overLimit);
    gettimeofday(&tv_end, NULL);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity_local(retMat);
    unsigned long long crc64 = mat_crc64_local(retMat);
    printf("00,%05d,%03d,%f,%f,%016llx,%016llx\n", size, 1, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(tmp);
    return time_spent;
}

double bmark_mpi(int size, double precision) {
    struct timeval tv_start, tv_end;
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);

    mat_t* mainMatrix = NULL;
    if(node == 0) {
        mainMatrix = mat_factory_init_seeded(size,size);
    }

    bool overLimit = true;

    MPI_Barrier(MPI_COMM_WORLD);
    gettimeofday(&tv_start, NULL);

    mat_t* local = mat_scatter(mainMatrix, size, size);
    mat_t *tmp = mat_init_clone_edge(local);
    dispatcher_task_t *dispatcher_task = dispatcher_task_init(local, tmp, precision, &overLimit);
    dispatcher_task_run(dispatcher_task);
    mat_t *retMat = dispatcher_task_mat(dispatcher_task);
    mat_gather(mainMatrix, retMat, size, size);
    mat_destroy(local);
    mat_destroy(tmp);
    gettimeofday(&tv_end, NULL);

    unsigned int loopCtr = dispatcher_task_loop_count(dispatcher_task);

    double local_time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                              + (double) (tv_end.tv_sec - tv_start.tv_sec);

    double global_max_time_spent;
    MPI_Reduce(&local_time_spent, &global_max_time_spent, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if(node == 0) {
        unsigned long long local_parity = mat_parity_local(mainMatrix);
        unsigned long long local_crc64 = mat_crc64_local(mainMatrix);
        printf("%08d,01,%05d,%03d,%f,%f,%016llx,%016llx\n", loopCtr, size, totalNodes,  precision, global_max_time_spent, local_parity, local_crc64);
    }

    dispatcher_task_destroy(dispatcher_task);

    if(node == 0) {
        mat_destroy(mainMatrix);
    }
    return local_time_spent;
}