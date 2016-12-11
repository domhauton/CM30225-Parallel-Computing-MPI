//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_MSG_MNGR_H
#define PARALLEL_COMPUTATION_CW2_MSG_MNGR_H

#include "../spool/spool.h"

typedef struct MSG_MNGR_T msg_mngr_t;
typedef struct MSG_MAT_ROW_T msg_mat_row_t;

msg_mngr_t *msg_mngr_init(spool_t* spool);

void msg_mngr_assign_rows(msg_mngr_t* msg_mngr, int rowIdStart, int rowIdEnd);

void msg_mngr_send_row(msg_mngr_t* msg_mngr, msg_mat_row_t* msg_mat_row);

void msg_mngr_broadcast_overflow(msg_mngr_t* msg_mngr, int smthr_rnd);

void msg_mngr_destroy(msg_mngr_t* msg_mngr);

#endif //PARALLEL_COMPUTATION_CW2_MSG_MNGR_H
