cmake_minimum_required(VERSION 2.8.12.2)
project(parallel_computation_cw2)

set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
set(CMAKE_C_COMPILER ${MPI_C_COMPILER})

find_package (MPI REQUIRED)
include_directories( ${MPI_INCLUDE_PATH} )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

set(CMAKE_C_FLAGS "-std=gnu99  -march=native -O2 -pthread -Wall ${CMAKE_C_FLAGS} ${MPI_COMPILE_FLAGS}")

set(SOURCE_FILES src/main.c src/matrix/mat.h src/matrix/mat.c src/matrix/mat_itr.c src/matrix/mat_itr.h src/smoothing/smoother.c src/smoothing/smoother.h src/matrix/mat_factory.c src/matrix/mat_factory.h src/benchmarking/bmark.h src/benchmarking/bmark.c src/spool/spool.c src/spool/spool.h src/debug.h src/spool/spool_dispatcher.c src/spool/spool_dispatcher.h src/smoothing/dispatcher.c src/smoothing/dispatcher.h)

add_executable(parallel_computation_cw2 ${SOURCE_FILES} src/matrix/mat.h src/matrix/mat.c src/smoothing/smoother.c src/smoothing/smoother.h src/benchmarking/bmark.h src/benchmarking/bmark.c src/spool/spool.c src/spool/spool.h src/debug.h src/spool/spool_dispatcher.c src/spool/spool_dispatcher.h src/smoothing/dispatcher.c src/smoothing/dispatcher.h)

target_link_libraries(parallel_computation_cw2 m)