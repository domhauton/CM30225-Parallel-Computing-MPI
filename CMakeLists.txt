cmake_minimum_required(VERSION 2.8.12.2)
project(parallel_computation_cw2)

set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
set(CMAKE_C_COMPILER ${MPI_C_COMPILER})

find_package (MPI REQUIRED)
include_directories( ${MPI_INCLUDE_PATH} )

set(CMAKE_C_FLAGS "-std=gnu99  -march=native -O2 -pthread -Wall ${CMAKE_C_FLAGS} ${MPI_COMPILE_FLAGS}")

set(SOURCE_FILES src/main.c src/matrix/mat.h src/matrix/mat.c src/matrix/mat_itr.c src/matrix/mat_itr.h src/matrix/mat_smthr.c src/matrix/mat_smthr.h src/matrix/mat_factory.c src/matrix/mat_factory.h src/matrix/bmark.h src/matrix/bmark.c src/spool/spool.c src/spool/spool.h src/debug.h src/messaging/msg_mngr.c src/messaging/msg_mngr.h src/spool/spool_dispatch.c src/spool/spool_dispatch.h)

add_executable(parallel_computation_cw2 ${SOURCE_FILES} src/matrix/mat.h src/matrix/mat.c src/matrix/mat_smthr.c src/matrix/mat_smthr.h src/matrix/bmark.h src/matrix/bmark.c src/spool/spool.c src/spool/spool.h src/debug.h src/messaging/msg_mngr.c src/messaging/msg_mngr.h src/spool/spool_dispatch.c src/spool/spool_dispatch.h)

target_link_libraries(parallel_computation_cw2 m)