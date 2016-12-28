//
// Created by dominic on 05/11/16.
//

#include <malloc.h>
#include "smoother.h"

/* Steps along iterators adding the outer values and placing them in target */
typedef struct smoother_T {
    mat_itr_t *target;
    mat_itr_t *srcMid;
    mat_itr_t *srcUp;
    mat_itr_t *srcDown;
    mat_itr_t *srcLeft;
    mat_itr_t *srcRight;
    //long ctr;
    double diffLimit;
    bool *overLimit;
} smoother_t;

/* Allocates and assigns values to a smoother_t
        Expects srcDown to reach then end of the matrix first.
        Next points to a second possible job and can be null */
smoother_t *smoother_init(mat_itr_t *target,
                            mat_itr_t *srcCenter,
                            mat_itr_t *srcUp,
                            mat_itr_t *srcDown,
                            mat_itr_t *srcLeft,
                            mat_itr_t *srcRight,
                            bool *overLimit,
                            double diffLimit) {
    smoother_t *matSmoother = malloc(sizeof(smoother_t));
    matSmoother->target = target;
    matSmoother->srcMid = srcCenter;
    matSmoother->srcUp = srcUp;
    matSmoother->srcDown = srcDown;
    matSmoother->srcLeft = srcLeft;
    matSmoother->srcRight = srcRight;
    matSmoother->diffLimit = diffLimit;
    matSmoother->overLimit = overLimit;
    return matSmoother;
}

/*
 * Clones a chain of smoother.
 */
smoother_t *smoother_clone(smoother_t* smoother_old) {
    smoother_t *matSmoother = malloc(sizeof(smoother_t));
    matSmoother->target = mat_itr_clone(smoother_old->target);
    matSmoother->srcMid = mat_itr_clone(smoother_old->srcMid);
    matSmoother->srcUp = mat_itr_clone(smoother_old->srcUp);
    matSmoother->srcDown = mat_itr_clone(smoother_old->srcDown);
    matSmoother->srcLeft = mat_itr_clone(smoother_old->srcLeft);
    matSmoother->srcRight = mat_itr_clone(smoother_old->srcRight);
    matSmoother->diffLimit = smoother_old->diffLimit;
    matSmoother->overLimit = smoother_old->overLimit;
    return matSmoother;
}

/* Averages all values bypassing diff calculations */
void smoother_smooth_ignore_diff(smoother_t *matSmoother) {
    while (mat_itr_hasNext(matSmoother->srcDown)) {
        *mat_itr_next(matSmoother->target) = ((*mat_itr_next(matSmoother->srcUp) +
                                               *mat_itr_next(matSmoother->srcDown)) +
                                              (*mat_itr_next(matSmoother->srcLeft) +
                                               *mat_itr_next(matSmoother->srcRight))) / 4;
    }
}

/* Averages all matrix values, checking if new value is over the diff */
void smoother_run(smoother_t *matSmoother) {
    while (!*matSmoother->overLimit && mat_itr_hasNext(matSmoother->srcDown)) {
        double *currentResPtr = mat_itr_next(matSmoother->target);
        *currentResPtr = ((*mat_itr_next(matSmoother->srcUp) +
                           *mat_itr_next(matSmoother->srcDown)) +
                          (*mat_itr_next(matSmoother->srcLeft) +
                           *mat_itr_next(matSmoother->srcRight))) / 4;
        double diff = *currentResPtr - *mat_itr_next(matSmoother->srcMid);
        if (matSmoother->diffLimit < diff && -diff < matSmoother->diffLimit) {
            *matSmoother->overLimit = true;
        }
    }

    smoother_smooth_ignore_diff(matSmoother);
}

/* Removes a smoother_t.
        Undefined behaviour if smoother_t used after removal. */
void smoother_destroy(smoother_t *matSmoother) {
    mat_itr_destroy(matSmoother->srcMid);
    mat_itr_destroy(matSmoother->srcUp);
    mat_itr_destroy(matSmoother->srcDown);
    mat_itr_destroy(matSmoother->srcLeft);
    mat_itr_destroy(matSmoother->srcRight);
    mat_itr_destroy(matSmoother->target);
    free(matSmoother);
}