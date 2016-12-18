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
#include "../spool/spool_task.h"
#include "../spool/spool_task_runner.h"

double bmark_serial(int size, double precision) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    bool *overLimit = malloc(sizeof(int));
    gettimeofday(&tv_start, NULL);
    mat_t *retMat = mat_smooth(matrix1, matrix2, precision, overLimit);
    gettimeofday(&tv_end, NULL);
    free(overLimit);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    printf("00,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", size, 1, 1, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(matrix2);
    return time_spent;
}

double bmark_pool(int size, double precision, unsigned int threads, unsigned int cut) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    bool overLimit = true;
    spool_t *mainSpool = spool_init(NULL, threads);
    spool_task_t *spool_task = spool_task_init(matrix1, matrix2, precision, &overLimit, cut, NULL);
    spool_t *dispatcher = spool_init(mainSpool, spool_task);

    gettimeofday(&tv_start, NULL);
    spool_run(dispatcher);
    gettimeofday(&tv_end, NULL);

    mat_t *retMat = spool_task_mat(spool_task);
    unsigned int loopCtr = spool_task_loop_count(spool_task);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    int chunks = (size-2)/cut + ((size-2)%cut == 0 ? 0 : 1);

    printf("%08d,01,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", loopCtr, size, threads, chunks, precision, time_spent, parity, crc64);

    spool_task_destroy(spool_task);
    spool_destroy(dispatcher);
    spool_destroy(mainSpool);
    mat_destroy(matrix1);
    mat_destroy(matrix2);

    return time_spent;
}

