//
// Created by dominic on 05/11/16.
//

#include "mat.h"

#ifndef PARALLEL_COMPUTATION_CW1_MATRIXFACTORY_H
#define PARALLEL_COMPUTATION_CW1_MATRIXFACTORY_H

mat_t *mat_factory_init_empty(long xSize, long ySize);

mat_t *mat_factory_init_seeded(int xSize, int ySize);

#endif //PARALLEL_COMPUTATION_CW1_MATRIXFACTORY_H
