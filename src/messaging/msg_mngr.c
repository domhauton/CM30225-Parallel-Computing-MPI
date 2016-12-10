//
// Created by dominic on 08/12/16.
//

#include "msg_mngr.h"

typedef struct MSG_MNGR_RND_SYNC_T {
    pthread_cond_t endOfJob;
    pthread_mutex_t countLock;
    unsigned int count;
} msg_mngr_rnd_sync_t;