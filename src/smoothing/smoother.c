//
// Created by dominic on 05/11/16.
//

#include <malloc.h>
#include <immintrin.h>
#include <math.h>
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
 * Clones a smoothers.
 */
smoother_t *smoother_clone(smoother_t *smoother_old) {
    smoother_t *matSmoother = malloc(sizeof(smoother_t));
    // All iterators must be cloned.
    matSmoother->target = mat_itr_clone(smoother_old->target);
    matSmoother->srcMid = mat_itr_clone(smoother_old->srcMid);
    matSmoother->srcUp = mat_itr_clone(smoother_old->srcUp);
    matSmoother->srcDown = mat_itr_clone(smoother_old->srcDown);
    matSmoother->srcLeft = mat_itr_clone(smoother_old->srcLeft);
    matSmoother->srcRight = mat_itr_clone(smoother_old->srcRight);
    // Other values copied. Must use same memory locations
    matSmoother->diffLimit = smoother_old->diffLimit;
    matSmoother->overLimit = smoother_old->overLimit;
    return matSmoother;
}

/* Averages all values bypassing diff calculations */
void smoother_smooth_ignore_diff(smoother_t *matSmoother) {
    while (mat_itr_hasNext(matSmoother->srcDown)) {
        if (mat_itr_swar_capable(matSmoother->srcDown)) {
            // Load SIMD registers
            __m256d up = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcUp));
            __m256d down = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcDown));
            __m256d left = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcLeft));
            __m256d right = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcRight));
            // SIMD Calculation
            __m256d res = _mm256_div_pd(_mm256_add_pd(_mm256_add_pd(up, down), _mm256_add_pd(left, right)), _mm256_set1_pd(4.0f));
            // SIMD Dump
            _mm256_storeu_pd(mat_itr_next_swar(matSmoother->target), res);
        } else {
            // Standard non-SIMD calculation. Brackets help GCC Pipeline
            *mat_itr_next(matSmoother->target) = ((*mat_itr_next(matSmoother->srcUp) +
                                                   *mat_itr_next(matSmoother->srcDown)) +
                                                  (*mat_itr_next(matSmoother->srcLeft) +
                                                   *mat_itr_next(matSmoother->srcRight))) / 4;
        }
    }
}

/* Averages all matrix values, checking if new value is over the diff */
void smoother_run(smoother_t *matSmoother) {
    while (!*matSmoother->overLimit && mat_itr_hasNext(matSmoother->srcDown)) {
        if (mat_itr_swar_capable(matSmoother->srcDown)) {
            // Load SIMD Registers
            __m256d up = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcUp));
            __m256d down = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcDown));
            __m256d left = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcLeft));
            __m256d right = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcRight));
            // SIMD Calculation
            __m256d res = _mm256_div_pd(_mm256_add_pd(_mm256_add_pd(up, down), _mm256_add_pd(left, right)), _mm256_set1_pd(4.0f));
            // Dump SIMD Result to memory
            _mm256_storeu_pd(mat_itr_next_swar(matSmoother->target), res);

            // Load SIMD Registers from old matrix
            __m256d old = _mm256_loadu_pd(mat_itr_next_swar(matSmoother->srcMid));
            // Find the diff from old values in SIMD
            __m256d posDiff = _mm256_sub_pd(res, old);
            __m256d negDiff = _mm256_sub_pd(_mm256_set1_pd(0.0f), posDiff);
            // Find the largest diff
            __m256d absDiff = _mm256_max_pd(posDiff, negDiff);

            // SIMD comparison to diff limit and reduce into boolean
            if(_mm256_movemask_pd(_mm256_cmp_pd(absDiff, _mm256_set1_pd(matSmoother->diffLimit), _CMP_GT_OQ))){
                *matSmoother->overLimit = true;
            }

        } else {
            // Standard non-SIMD calculation. Brackets help GCC Pipeline
            double *currentResPtr = mat_itr_next(matSmoother->target);
            *currentResPtr = ((*mat_itr_next(matSmoother->srcUp) +
                               *mat_itr_next(matSmoother->srcDown)) +
                              (*mat_itr_next(matSmoother->srcLeft) +
                               *mat_itr_next(matSmoother->srcRight))) / 4;
            // Diff calculation for smoother.
            double absDiff = fabs(*currentResPtr - *mat_itr_next(matSmoother->srcMid));
            if (absDiff > matSmoother->diffLimit) {
                *matSmoother->overLimit = true;
            }
        }
    }

    smoother_smooth_ignore_diff(matSmoother);
}

/* Removes a smoother_t.
        Undefined behaviour if smoother_t used after removal. */
void smoother_destroy(smoother_t *matSmoother) {
    // Clean up all the iterators
    mat_itr_destroy(matSmoother->srcMid);
    mat_itr_destroy(matSmoother->srcUp);
    mat_itr_destroy(matSmoother->srcDown);
    mat_itr_destroy(matSmoother->srcLeft);
    mat_itr_destroy(matSmoother->srcRight);
    mat_itr_destroy(matSmoother->target);
    free(matSmoother);
}