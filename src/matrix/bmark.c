//
// Created by dominic on 16/11/16.
//

#include <bits/time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include "mat.h"
#include "mat_factory.h"
#include "mat_smooth_parallel.h"
#include "bmark.h"

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

double bmark_unpooled(int size, double precision, int threads) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    gettimeofday(&tv_start, NULL);
    mat_t *retMat = mat_smooth_parallel_join(matrix1, matrix2, precision, threads);
    gettimeofday(&tv_end, NULL);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    printf("01,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", size, threads, threads, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(matrix2);
    return time_spent;
}

double bmark_barrier_leapfrog(int size, double precision, unsigned int threads) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    gettimeofday(&tv_start, NULL);
    mat_t *retMat = mat_smooth_parallel_barrier_leapfrog(matrix1, matrix2, precision, threads);
    gettimeofday(&tv_end, NULL);
    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    printf("02,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", size, threads, threads, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(matrix2);
    return time_spent;
}

double bmark_barrier_rowcut(int size, double precision, unsigned int threads) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    gettimeofday(&tv_start, NULL);
    mat_t *retMat = mat_smooth_parallel_barrier_rowcut(matrix1, matrix2, precision, threads);
    gettimeofday(&tv_end, NULL);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    printf("03,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", size, threads, threads, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(matrix2);

    return time_spent;
}

double bmark_pool_rowcut(int size, double precision, unsigned int threads, unsigned int cut) {
    struct timeval tv_start, tv_end;

    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    gettimeofday(&tv_start, NULL);
    mat_t *retMat = mat_smooth_parallel_pool_rowcut(matrix1, matrix2, precision, threads, cut);
    gettimeofday(&tv_end, NULL);

    double time_spent = (double) (tv_end.tv_usec - tv_start.tv_usec) / 1000000
                        + (double) (tv_end.tv_sec - tv_start.tv_sec);

    unsigned long long parity = mat_parity(retMat);
    unsigned long long crc64 = mat_crc64(retMat);

    int chunks = (size-2)/cut + ((size-2)%cut == 0 ? 0 : 1);

    printf("04,%05d,%03d,%04d,%f,%f,%016llx,%016llx\n", size, threads, chunks, precision, time_spent, parity, crc64);

    mat_destroy(matrix1);
    mat_destroy(matrix2);

    return time_spent;
}