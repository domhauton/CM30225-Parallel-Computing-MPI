//
// Created by dominic on 01/11/16.
//

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

/* Iterator implementation for mat_t */
typedef struct MAT_ITR_T {
    double *currentPtr;
    double *rowEnd;
    double *areaEnd;
    long nextRowJump;
    long areaWidth;
} mat_itr_t;

/* Will iterate over every value on the outside of the matrix */
typedef struct MAT_ITR_EDGE_T {
    double *currentPtr;
    double *endFirstRow;
    double *startFinalRow;
    double *endFinalRow;
    // Navigation
    bool isAtRowStart;
    long rowJumpSize;
} mat_itr_edge_t;

/* Moves the iterator pointer down one row.
        Always returns true */
bool mat_itr_advance_row(mat_itr_t *matIterator) {
    matIterator->currentPtr += matIterator->nextRowJump;
    matIterator->rowEnd += matIterator->nextRowJump + matIterator->areaWidth;
    return true;
}

/* Initialises an iterator for a mat_t.
        Incorrect parameters will result in undefined behaviour */
mat_itr_t *mat_itr_init(double *dataPtr, long fullWidth, long areaWidth, long areaHeight) {
    mat_itr_t *matIterator = malloc(sizeof(mat_itr_t));
    matIterator->currentPtr = dataPtr;
    matIterator->nextRowJump = fullWidth - areaWidth;
    matIterator->areaEnd = matIterator->currentPtr + (fullWidth * (areaHeight - 1)) + areaWidth;
    matIterator->areaWidth = areaWidth;
    matIterator->rowEnd = matIterator->currentPtr + matIterator->areaWidth - 1;
    return matIterator;
}

mat_itr_t *mat_itr_clone(mat_itr_t* mat_itr_old) {
    mat_itr_t *matIterator = malloc(sizeof(mat_itr_t));
    matIterator->currentPtr = mat_itr_old->currentPtr;
    matIterator->nextRowJump = mat_itr_old->nextRowJump;
    matIterator->areaEnd = mat_itr_old->areaEnd;
    matIterator->areaWidth = mat_itr_old->areaWidth;
    matIterator->rowEnd = mat_itr_old->rowEnd;
    return matIterator;
}

/* True if the iterator has more values */
bool mat_itr_hasNext(mat_itr_t *matIterator) {
    return matIterator != NULL && matIterator->currentPtr < matIterator->areaEnd;
}

/* True if the iterator has more values */
bool mat_itr_edge_hasNext(mat_itr_edge_t *edgeIterator) {
    return edgeIterator->currentPtr < edgeIterator->endFinalRow;
}

/* Advance one value in the matrix.
        Behaviour undefined if used on a split matrix
        Behaviour undefined if end of matrix is passed */
double *mat_itr_next(mat_itr_t *matIterator) {
    if (matIterator->currentPtr > matIterator->rowEnd) {
        mat_itr_advance_row(matIterator);
    }
    return matIterator->currentPtr++;
}

/* Closes the iterator. */
void mat_itr_destroy(mat_itr_t *matIterator) {
    free(matIterator);
}

/* Returns a matrix edge iterator */
mat_itr_edge_t *mat_itr_edge_init(double *data, long width, long height) {
    mat_itr_edge_t *edgeIterator = malloc(sizeof(mat_itr_edge_t));
    edgeIterator->currentPtr = data - 1;
    edgeIterator->endFirstRow = data + width - 1;
    edgeIterator->startFinalRow = data + (width * (height - 1));
    edgeIterator->endFinalRow = data + (height * width) - 1;
    edgeIterator->isAtRowStart = true;
    edgeIterator->rowJumpSize = width - 1;
    return edgeIterator;
}

/* Advances the iterator to a unseen value on the edge and returns the value */
double *mat_itr_edge_next(mat_itr_edge_t *edgeIterator) {
    if (edgeIterator->currentPtr <= edgeIterator->endFirstRow) {
        return ++edgeIterator->currentPtr;
    } else if (edgeIterator->currentPtr < edgeIterator->startFinalRow) {
        if (edgeIterator->isAtRowStart) {
            edgeIterator->isAtRowStart = false;
            edgeIterator->currentPtr += edgeIterator->rowJumpSize;
            return edgeIterator->currentPtr;
        } else {
            edgeIterator->isAtRowStart = true;
            return ++edgeIterator->currentPtr;
        }
    } else if (edgeIterator->currentPtr < edgeIterator->endFinalRow) {
        return ++edgeIterator->currentPtr;
    } else {
        return NULL;
    }
}

/* Cleans up the structure. Do not use after calling this function. */
void mat_itr_edge_destroy(mat_itr_edge_t *edgeIterator) {
    free(edgeIterator);
}





