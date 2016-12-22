//
// Created by dominic on 16/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_BENCHMARK_H
#define PARALLEL_COMPUTATION_CW1_BENCHMARK_H

double bmark_serial(int size, double precision);

double bmark_mpi(int size, double precision);

double bmark_mpi_pool(int size, double precision, unsigned int threads, unsigned int cut);

#endif //PARALLEL_COMPUTATION_CW1_BENCHMARK_H
