//
// Created by dominic on 08/12/16.
//

#include <wchar.h>
#include "msg_mngr.h"
#include "../spool/spool_task_runner.h"
#include "../matrix/mat_factory.h"

typedef enum MSG_TYPE {INIT, MAT, OVERFLOW, SYN, FIN} msg_type;

typedef struct MSG_T {
    msg_type type;
    int jobId;
    void* content;
};

typedef struct MSG_MNGR_JOB_T {
    mat_t *srcMat;
    mat_t *tmpMat;
    spool_task_t* spool_task;
    spool_t *dispatcher;
    msg_mngr_job_t *next;
} msg_mngr_job_t;
6
typedef struct MSG_MNGR_T {
    spool_t* spool;
} msg_mngr_t;

msg_mngr_job_t *msg_mngr_job_init(msg_mngr_t *msg_mngr, int size, double precision, double cut) {
    mat_t *matrix1 = mat_factory_init_random(size, size);
    mat_t *matrix2 = mat_factory_init_random(size, size);
    bool overLimit = true;
    spool_t *mainSpool = spool_init(NULL, threads);
    spool_task_t *spool_task = spool_task_init(matrix1, matrix2, precision, &overLimit, CUT_SIZE, NULL);
    spool_t *dispatcher = spool_init(mainSpool, spool_task);
}

