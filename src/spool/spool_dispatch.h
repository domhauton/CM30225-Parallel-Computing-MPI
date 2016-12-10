//
// Created by dominic on 08/12/16.
//

#ifndef PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
#define PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H

#include "../matrix/mat.h"

mat_t *spool_dispatch_local(mat_t *source,
                            mat_t *target,
                            double limit,
                            unsigned int *loopCtr,
                            unsigned int threadCount,
                            unsigned int smthrSize);

#endif //PARALLEL_COMPUTATION_CW2_SPOOL_DISPATCH_H
