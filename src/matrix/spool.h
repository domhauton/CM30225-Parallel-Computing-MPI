//
// Created by dominic on 19/11/16.
//

#ifndef PARALLEL_COMPUTATION_CW1_SMOOTHINGPOOL_H
#define PARALLEL_COMPUTATION_CW1_SMOOTHINGPOOL_H

#include <semaphore.h>
#include "mat_smthr.h"

typedef struct SPOOL_JOB_SYNC_T spool_job_sync_t;
typedef struct SPOOL_JOB_T spool_job_t;
typedef struct SPOOL_THREAD_T spool_thread_t;
typedef struct SPOOL_T spool_t;

spool_t *spool_init(spool_job_t *jobList);

void spool_worker_add(spool_t *spool);

void spool_job_add(spool_t *spool, mat_smthr_t *smoother, spool_job_sync_t *spoolJobSync);

spool_job_sync_t *spool_job_add_batch(spool_t *spool, mat_smthr_list_t *smthrList, unsigned int count);

void spool_job_sync_wait(spool_job_sync_t *spoolJobSync);

void spool_worker_remove(spool_t *spool);

void spool_job_sync_destroy(spool_job_sync_t *spoolJobSync);

spool_job_t *spool_destroy(spool_t *spool);

#endif //PARALLEL_COMPUTATION_CW1_SMOOTHINGPOOL_H
