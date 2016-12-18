//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_MSG_MNGR_H
#define PARALLEL_COMPUTATION_CW2_MSG_MNGR_H

#define CUT_SIZE 20;

#include "../spool/spool.h"
#include "../matrix/mat.h"
#include "../spool/spool_task.h"

typedef struct MSG_MNGR_T msg_mngr_t;
typedef struct MSG_MNGR_JOB_T msg_mngr_job_t;

msg_mngr_t *msg_mngr_init(spool_t* spool);

void msg_mngr_run_listener(msg_mngr_t* msg_mngr);

void msg_mngr_assign_rows(msg_mngr_t* msg_mngr, int rowIdStart, int rowIdEnd);

void msg_mngr_send_mat_section(msg_mngr_t* msg_mngr, mat_t* msg_mat_row);

void msg_mngr_broadcast_overflow(msg_mngr_t* msg_mngr, int smthr_rnd);

void msg_mngr_task_complete(spool_task_t *spool_task);

void msg_mngr_destroy(msg_mngr_t* msg_mngr);

#endif //PARALLEL_COMPUTATION_CW2_MSG_MNGR_H
