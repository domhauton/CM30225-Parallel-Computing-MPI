cmake_minimum_required(VERSION 2.8.12.2)
project(parallel_computation_cw1)


set(CMAKE_C_FLAGS "-std=gnu99  -march=native -O2 -pthread -Wall ${CMAKE_C_FLAGS}")

set(SOURCE_FILES src/main.c src/matrix/mat.h src/matrix/mat.c src/matrix/mat_itr.c src/matrix/mat_itr.h src/matrix/mat_smthr.c src/matrix/mat_smthr.h src/matrix/mat_factory.c src/matrix/mat_factory.h src/matrix/mat_smooth_parallel.c src/matrix/mat_smooth_parallel.h src/matrix/bmark.h src/matrix/bmark.c src/matrix/spool.c src/matrix/spool.h src/debug.h)

add_executable(parallel_computation_cw1 ${SOURCE_FILES} src/matrix/mat.h src/matrix/mat.c src/matrix/mat_smthr.c src/matrix/mat_smthr.h src/matrix/mat_smooth_parallel.c src/matrix/mat_smooth_parallel.h src/matrix/bmark.h src/matrix/bmark.c src/matrix/spool.c src/matrix/spool.h src/debug.h)

target_link_libraries(parallel_computation_cw1 m)