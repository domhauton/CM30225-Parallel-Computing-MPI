//
// Created by dominic on 08/12/16.
//

#include <stdlib.h>
#include "spool_dispatch.h"
#include "spool.h"


mat_t *spool_dispatch_local(mat_t *source,
                            mat_t *target,
                            double limit,
                            unsigned int *loopCtr,
                            unsigned int threadCount,
                            unsigned int smthrSize) {
    bool resultFlipped = false;
    mat_t *tmp;
    bool *overLimit = malloc(sizeof(int));
    spool_t *spool = spool_init(NULL);

    for(int i = 0; i < threadCount; i++) {
        spool_worker_add(spool);
    }

    mat_smthr_list_t *jobRoot = malloc(sizeof(mat_smthr_list_t));
    jobRoot->data = NULL;

    do {
        *overLimit = false;
        // Submit Jobs
        long jobCount = mat_smthr_get(source, target, limit, overLimit, smthrSize, jobRoot);
        spool_job_sync_t *spoolJobSync = spool_job_add_batch(spool, jobRoot->next, (unsigned int) jobCount);
        spool_job_sync_wait(spoolJobSync);
        spool_job_sync_destroy(spoolJobSync);
        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;
        (*loopCtr)++;
    } while (*overLimit);

    spool_destroy(spool);
    free(jobRoot);
    free(overLimit);

    return resultFlipped ? source : target;
}