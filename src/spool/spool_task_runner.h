//
// Created by dominic on 12/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_SPOOL_H
#define PARALLEL_COMPUTATION_CW2_SPOOL_H

#include "spool_task.h"

typedef struct SPOOL_T spool_t;

spool_t *spool_init(spool_t* spool, spool_task_t *spool_task, unsigned int id);

void spool_run(spool_t *dispatcher);

void spool_destroy(spool_t *dispatcher);

#endif //PARALLEL_COMPUTATION_CW2_SPOOL_H
