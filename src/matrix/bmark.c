//
// Created by dominic on 16/11/16.
//

#include <bits/time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mat.h"
#include "mat_factory.h"
#include "bmark.h"
#include "../spool/dispatcher.h"

double bmark_serial(int size, double precision) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_seeded(size, size);
    mat_t *tmp = mat_init_clone_edge(matrix1);
    bool overLimit;
    gettimeofday(&tv_start, NULL);
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

double bmark_pool(int size, double precision, unsigned int threads, unsigned int cut) {
    struct timeval tv_start, tv_end;
    int node, totalNodes;
    MPI_Comm_rank(MPI_COMM_WORLD, &node);
    MPI_Comm_size(MPI_COMM_WORLD, &totalNodes);

    mat_t* main = NULL;
    if(node == 0) {
        main = mat_factory_init_seeded(size,size);
    }
    mat_t* local = mat_scatter(main, size, size);
    mat_t *tmp = mat_init_clone_edge(local);

    bool overLimit = true;
    spool_t *mainSpool = spool_init(NULL, threads);
    dispatcher_task_t *dispatcher_task = dispatcher_task_init(local, tmp, precision, &overLimit, cut);

    gettimeofday(&tv_start, NULL);
    dispatcher_task_run(dispatcher_task, mainSpool);
    gettimeofday(&tv_end, NULL);

    mat_t *retMat = dispatcher_task_mat(dispatcher_task);
    unsigned int loopCtr = dispatcher_task_loop_count(dispatcher_task);



    double local_time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    double global_max_time_spent;
    MPI_Reduce(&local_time_spent, &global_max_time_spent, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    if(node == 0) {
        printf("%08d,01,%05d,%03d,%f,%f,%016llx,%016llx\n", loopCtr, size, threads,  precision, global_max_time_spent, parity, crc64);
    }
    dispatcher_task_destroy(dispatcher_task);
    spool_destroy(mainSpool);

    mat_destroy(local);
    mat_destroy(tmp);
    if(node == 0) {
        mat_destroy(main);
    }
    return local_time_spent;
}

