//
// Created by dominic on 05/11/16.
//

#include <malloc.h>
#include "mat_smthr.h"

/* Steps along iterators adding the outer values and placing them in target */
typedef struct MAT_SMTHR_T {
    mat_itr_t *target;
    mat_itr_t *srcMid;
    mat_itr_t *srcUp;
    mat_itr_t *srcDown;
    mat_itr_t *srcLeft;
    mat_itr_t *srcRight;
    //long ctr;
    double diffLimit;
    bool *overLimit;
    bool split;
} mat_smthr_t;

/* Allocates and assigns values to a mat_smthr_t
        Expects srcDown to reach then end of the matrix first */
mat_smthr_t *mat_smthr_init(mat_itr_t *target,
                            mat_itr_t *srcCenter,
                            mat_itr_t *srcUp,
                            mat_itr_t *srcDown,
                            mat_itr_t *srcLeft,
                            mat_itr_t *srcRight,
                            bool *overLimit,
                            double diffLimit) {
    mat_smthr_t *matSmoother = malloc(sizeof(mat_smthr_t));
    matSmoother->target = target;
    matSmoother->srcMid = srcCenter;
    matSmoother->srcUp = srcUp;
    matSmoother->srcDown = srcDown;
    matSmoother->srcLeft = srcLeft;
    matSmoother->srcRight = srcRight;
    matSmoother->diffLimit = diffLimit;
    matSmoother->overLimit = overLimit;
    matSmoother->split = false;
    return matSmoother;
}

/* Produces a new mat_smthr_t by splitting all iterators */
mat_smthr_t *mat_smthr_split(mat_smthr_t *matSmoother1) {
    mat_smthr_t *matSmoother2 = malloc(sizeof(mat_smthr_t));
    matSmoother2->target = mat_itr_split(matSmoother1->target);
    matSmoother2->srcMid = mat_itr_split(matSmoother1->srcMid);
    matSmoother2->srcUp = mat_itr_split(matSmoother1->srcUp);
    matSmoother2->srcDown = mat_itr_split(matSmoother1->srcDown);
    matSmoother2->srcLeft = mat_itr_split(matSmoother1->srcLeft);
    matSmoother2->srcRight = mat_itr_split(matSmoother1->srcRight);
    matSmoother2->diffLimit = matSmoother1->diffLimit;
    matSmoother2->overLimit = matSmoother1->overLimit;
    matSmoother2->split = true;
    matSmoother1->split = true;

    return matSmoother2;
}

/* Averages all values bypassing diff calculations */
void mat_smthr_smooth_ignore_diff(mat_smthr_t *matSmoother) {
    if (matSmoother->split) {
        while (mat_itr_hasNext(matSmoother->srcDown)) {
            *mat_itr_next_unsafe(matSmoother->target) = ((*mat_itr_next_unsafe(matSmoother->srcUp) +
                                                             *mat_itr_next_unsafe(matSmoother->srcDown)) +
                                                            (*mat_itr_next_unsafe(matSmoother->srcLeft) +
                                                             *mat_itr_next_unsafe(matSmoother->srcRight))) / 4;
        }
    } else {
        while (mat_itr_hasNext(matSmoother->srcDown)) {
            *mat_itr_step_unsafe(matSmoother->target) = ((*mat_itr_step_unsafe(matSmoother->srcUp) +
                                                             *mat_itr_step_unsafe(matSmoother->srcDown)) +
                                                            (*mat_itr_step_unsafe(matSmoother->srcLeft) +
                                                             *mat_itr_step_unsafe(matSmoother->srcRight))) / 4;
        }
    }
}

/* Averages all matrix values, checking if new value is over the diff */
void mat_smthr_smooth(mat_smthr_t *matSmoother) {
    if (matSmoother->split) {
        while (!*matSmoother->overLimit && mat_itr_hasNext(matSmoother->srcDown)) {
            double *currentResPtr = mat_itr_next_unsafe(matSmoother->target);
            *currentResPtr = ((*mat_itr_next_unsafe(matSmoother->srcUp) +
                               *mat_itr_next_unsafe(matSmoother->srcDown)) +
                              (*mat_itr_next_unsafe(matSmoother->srcLeft) +
                               *mat_itr_next_unsafe(matSmoother->srcRight))) / 4;
            double diff = *currentResPtr - *mat_itr_next_unsafe(matSmoother->srcMid);
            if (matSmoother->diffLimit < diff && -diff < matSmoother->diffLimit) {
                *matSmoother->overLimit = true;
            }
        }
    } else {
        while (!*matSmoother->overLimit && mat_itr_hasNext(matSmoother->srcDown)) {
            double *currentResPtr = mat_itr_step_unsafe(matSmoother->target);
            *currentResPtr = ((*mat_itr_step_unsafe(matSmoother->srcUp) +
                               *mat_itr_step_unsafe(matSmoother->srcDown)) +
                              (*mat_itr_step_unsafe(matSmoother->srcLeft) +
                               *mat_itr_step_unsafe(matSmoother->srcRight))) / 4;
            double diff = *currentResPtr - *mat_itr_step_unsafe(matSmoother->srcMid);
            if (matSmoother->diffLimit < diff && -diff < matSmoother->diffLimit) {
                *matSmoother->overLimit = true;
            }
        }
    }

    mat_smthr_smooth_ignore_diff(matSmoother);
}

/* Removes a mat_smthr_t.
        Undefined behaviour if mat_smthr_t used after removal. */
void mat_smthr_destroy(mat_smthr_t *matSmoother) {
    mat_itr_destroy(matSmoother->srcMid);
    mat_itr_destroy(matSmoother->srcUp);
    mat_itr_destroy(matSmoother->srcDown);
    mat_itr_destroy(matSmoother->srcLeft);
    mat_itr_destroy(matSmoother->srcRight);
    mat_itr_destroy(matSmoother->target);
    free(matSmoother);
}