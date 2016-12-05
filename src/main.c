#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "matrix/mat.h"
#include "matrix/mat_factory.h"
#include "matrix/bmark.h"
#include "matrix/spool.h"
#include "matrix/mat_smooth_parallel.h"

void matrixComputeTest() {
    mat_t *matrix = mat_factory_init_empty(10, 10);
    mat_t *matrix2 = mat_factory_init_empty(10, 10);
    mat_itr_edge_t *matEdgeIterator = mat_itr_edge_create(matrix);
    while (mat_itr_edge_hasNext(matEdgeIterator)) {
        *mat_itr_edge_next(matEdgeIterator) = 2;
    }
    mat_itr_edge_destroy(matEdgeIterator);

    mat_copy_edge(matrix, matrix2);
    mat_print(matrix);
    bool *overLimit = calloc(1, sizeof(bool));
    mat_t *res = mat_smooth(matrix, matrix2, 0.000001, overLimit);
    free(overLimit);
    printf("\n");
    mat_print(res);
    unsigned long long matPar = mat_parity(matrix);
    printf("Parity 1: %016llx\n", matPar);
    unsigned long long matCRC = mat_crc64(matrix2);
    printf("CRC64: %016llx\n", matCRC);

    mat_destroy(matrix);
    mat_destroy(matrix2);
}

void matrixMultiIteratorOverwriteTest() {
    mat_t *matrix = mat_factory_init_empty(7, 7);
    mat_itr_t *matIterator1 = mat_itr_create_partial(matrix, 1, 1, 5, 5);
    mat_itr_t *matIterator2 = mat_itr_split(matIterator1);
    mat_itr_t *matIterator3 = mat_itr_split(matIterator2);
    while (mat_itr_hasNext(matIterator1)) {
        *mat_itr_next(matIterator1) = 1.111111;
    }
    while (mat_itr_hasNext(matIterator2)) {
        *mat_itr_next(matIterator2) = 2.222222;
    }
    while (mat_itr_hasNext(matIterator3)) {
        *mat_itr_next(matIterator3) = 3.333333;
    }
    mat_itr_destroy(matIterator1);
    mat_itr_destroy(matIterator2);
    mat_itr_destroy(matIterator3);
    mat_print(matrix);
    printf("Parity: %016llx", mat_parity(matrix));
    mat_destroy(matrix);
}


void matrixCompareRegeneration(int size) {
    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    if (mat_equals(matrix1, matrix2)) {
        printf("Matrices same\n");
    } else {
        printf("Matrices not same\n");
    }

    mat_itr_t *matIterator = mat_itr_create_partial(matrix1, 0, 0, 1, 1);
    (*mat_itr_next(matIterator))++;
    mat_itr_destroy(matIterator);

    if (mat_equals(matrix1, matrix2)) {
        printf("Matrices same\n");
    } else {
        printf("Matrices not same\n");
    }

    mat_destroy(matrix1);
    mat_destroy(matrix2);
}

void testSpool() {
    // Enable Debug
    spool_t *spool = spool_init(NULL);
    spool_worker_add(spool);
    spool_worker_add(spool);
    sleep(1);
    spool_worker_remove(spool);
    sleep(1);
    spool_destroy(spool);
}

void testSplit() {
    // Enable Debug
    long size = 100;
    mat_t *matrix1 = mat_factory_init_random(size, size);
    bool *overLimit = calloc(1, sizeof(bool));
    mat_smthr_list_t *matSmthrList = malloc(sizeof(mat_smthr_list_t));
    mat_smthr_create_inner_cut_even(matrix1, matrix1, 0.001f, overLimit, 5, matSmthrList);
    mat_destroy(matrix1);
}

void matSmoothPoolRowCut() {
    mat_t *matrix = mat_factory_init_empty(10, 10);
    mat_t *matrix2 = mat_factory_init_empty(10, 10);
    mat_itr_edge_t *matEdgeIterator = mat_itr_edge_create(matrix);
    while (mat_itr_edge_hasNext(matEdgeIterator)) {
        *mat_itr_edge_next(matEdgeIterator) = 2;
    }
    mat_itr_edge_destroy(matEdgeIterator);

    mat_copy_edge(matrix, matrix2);
    mat_t *res = mat_smooth_parallel_pool_rowcut(matrix, matrix2, 0.000001, 30, 1);
    printf("\n");
    mat_print(res);
    unsigned long long matCRC = mat_crc64(matrix2);
    printf("Parity 1: %016llx\n", matCRC);

    mat_destroy(matrix);
    mat_destroy(matrix2);
}

int main(int argc, char *argv[]) {
    if (argc == 6) {
        unsigned int threads = (unsigned int) atoi(argv[1]);
        int size = atoi(argv[2]);
        double precision = strtof(argv[3], NULL);
        int type = atoi(argv[4]);
        unsigned int cut = (unsigned int) atoi(argv[5]);
        switch (type) {
            case 1:
                bmark_unpooled(size, precision, threads);
                break;
            case 2:
                bmark_barrier_leapfrog(size, precision, threads);
                break;
            case 3:
                bmark_barrier_rowcut(size, precision, threads);
                break;
            case 4:
                bmark_pool_rowcut(size, precision, threads, cut);
                break;
            default:
                bmark_serial(size, precision);
        }
    } else {
        printf("For individual calculations use: %s <threads> <size> <precision> <type> <cut>\n", argv[0]);
        printf("  Ctr   |Ty|Size |Thr|Chnk|  Acc   |  Time  |     Parity     |      CRC64     |\n");
        double precision = 0.0001f;
        for (unsigned int threads = 1; threads <= (sysconf(_SC_NPROCESSORS_ONLN) * 2); threads <<= 1) {
            for (int matrixSize = (2 << 7); matrixSize <= (2 << 10); matrixSize <<= 1) {
                bmark_serial(matrixSize, precision);
                bmark_unpooled(matrixSize, precision, threads);
                bmark_barrier_leapfrog(matrixSize, precision, threads);
                bmark_barrier_rowcut(matrixSize, precision, threads);
                bmark_pool_rowcut(matrixSize, precision, threads, 10);
            }
        }
    }
    return 0;
}




