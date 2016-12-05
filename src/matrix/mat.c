//
// Created by dominic on 31/10/16.
//

#include <stdlib.h>
#include <stdio.h>
#include "mat_itr.h"
#include "mat_smthr.h"
#include "../debug.h"

typedef struct MAT_T {
    long xSize, ySize;
    double *data;
} mat_t;

/* Creates the matrix and allocates values */
mat_t *mat_init(double *values, long xSize, long ySize) {
    mat_t *matrix = malloc(sizeof(mat_t));
    matrix->data = values;
    matrix->xSize = xSize;
    matrix->ySize = ySize;
    return matrix;
}

/* Returns a matrix data pointer at the given coordinates
        Out of bounds coordinates will result in undefined behaviour */
double *mat_data_ptr(mat_t *matrix, long x, long y) {
    return matrix->data + x + (matrix->xSize * y);
}

/* Returns a matrix iterator starting at the given co-ordinates for the given width and height
        Out of bounds area will result in undefined behaviour */
mat_itr_t *mat_itr_create_partial(mat_t *matrix, long x, long y, long width, long height) {
    double *initialDataPtr = mat_data_ptr(matrix, x, y);
    return mat_itr_init(initialDataPtr, matrix->xSize, width, height);
}

mat_itr_edge_t *mat_itr_edge_create(mat_t *matrix) {
    return mat_itr_edge_init(matrix->data, matrix->xSize, matrix->ySize);
}

/* Initialises a matrix smoother for an area starting at the given co-ordinates for the given width and height
        Will smooth until doubles do not surpass limit.
        overLimit may be shared between multiple smoothers (thread safe).
        target Matrix *must* have same dimensions as source.
        */
mat_smthr_t *mat_smthr_create_partial(mat_t *source, mat_t *tmp,
                                      double limit, bool *overLimit,
                                      long x, long y,
                                      long xSize, long ySize) {
    mat_itr_t *resItr = mat_itr_create_partial(tmp, x, y, xSize, ySize);
    mat_itr_t *ctrItr = mat_itr_create_partial(source, x, y, xSize, ySize);
    mat_itr_t *topItr = mat_itr_create_partial(source, x - 1, y, xSize, ySize);
    mat_itr_t *botItr = mat_itr_create_partial(source, x + 1, y, xSize, ySize);
    mat_itr_t *lftItr = mat_itr_create_partial(source, x, y - 1, xSize, ySize);
    mat_itr_t *rgtItr = mat_itr_create_partial(source, x, y + 1, xSize, ySize);
    return mat_smthr_init(resItr, ctrItr, topItr, botItr, lftItr, rgtItr, overLimit, limit);
}

/* Initialise a smoother for the inside of the matrix (excluding outer edge) */
mat_smthr_t *mat_smthr_create_inner(mat_t *source, mat_t *target, double limit, bool *overLimit) {
    long itrWidth = target->xSize - 2;
    long itrHeight = target->ySize - 2;
    return mat_smthr_create_partial(source, target, limit, overLimit, 1, 1, itrWidth, itrHeight);
}

/* Initialise a smoothers for the inside of the matrix (excluding outer edge). Place them in the given **smoothersArray
        If smoothers size < sections behaviour undefined */
void mat_smthr_create_inner_rowcut(mat_t *source,
                                   mat_t *tmp,
                                   double limit,
                                   bool *overLimit,
                                   unsigned int sections,
                                   mat_smthr_t **smoothers){
    long smoothingHeight = tmp->ySize - 2;
    mat_smthr_t** tmpPtr = smoothers;

    if(smoothingHeight <= sections) { // 1 Thread per row
        for(int i = 1; i < tmp->ySize; i++) {
            *tmpPtr++ = mat_smthr_create_partial(source, tmp, limit, overLimit, 1, i, tmp->xSize - 2, 1);
        }
    } else {
        long sectionHeight = smoothingHeight / sections;
        long longerSections = smoothingHeight % sections;
        long nextHeight = 1;
        for(int i = 0; i < sections; i++) {
            if(i < longerSections) {
                *tmpPtr++ = mat_smthr_create_partial(source, tmp, limit, overLimit, 1, nextHeight, tmp->xSize - 2,
                                                     sectionHeight + 1);
                nextHeight += sectionHeight + 1;
            } else {
                *tmpPtr++ = mat_smthr_create_partial(source, tmp, limit, overLimit, 1, nextHeight, tmp->xSize - 2,
                                                     sectionHeight);
                nextHeight += sectionHeight;
            }
        }
    }
}

/* Adds inner cut smooth jobs to head of give linked list */
long mat_smthr_create_inner_cut_even(mat_t *source,
                                     mat_t *tmp,
                                     double limit,
                                     bool *overLimit,
                                     unsigned int smthrSize,
                                     mat_smthr_list_t *matSmthrLinkedList){
    long sections = (source->ySize - 2) / smthrSize;
    long remainderSection = (source->ySize - 2) % smthrSize;
    debug_print("Splitting %ld matrix into %ld section/s of %d row/s with an extra %ld row section.\n",
                source->ySize, sections, smthrSize, remainderSection);
    long nextHeight = 1;
    for(int i = 0; i < sections; i++){
        debug_print("Section - [%ld:%d]\n", nextHeight, smthrSize);
        mat_smthr_list_t *newMatSmthrItem = malloc(sizeof(mat_smthr_list_t));
        newMatSmthrItem->data = mat_smthr_create_partial(source, tmp,
                                                         limit, overLimit,
                                                         1, nextHeight,
                                                         source->xSize - 2, smthrSize);
        matSmthrLinkedList->next = newMatSmthrItem;
        matSmthrLinkedList = matSmthrLinkedList->next;
        nextHeight += smthrSize;
    }
    if(remainderSection != 0) {
        debug_print("Section - [%ld:%ld]\n", nextHeight, remainderSection);
        mat_smthr_list_t *newMatSmthrItem = malloc(sizeof(mat_smthr_list_t));
        newMatSmthrItem->data = mat_smthr_create_partial(source, tmp,
                                                         limit, overLimit,
                                                         1, nextHeight,
                                                         source->xSize - 2, remainderSection);
        matSmthrLinkedList->next = newMatSmthrItem;
        matSmthrLinkedList = matSmthrLinkedList->next;
    }
    matSmthrLinkedList->next = NULL;
    debug_print("Splitting Complete.\n");
    return remainderSection == 0 ? sections : sections + 1;
}

/* Smooths the inside (exclude outer edge) of the given matrix until it changes < limit. */
mat_t *mat_smooth(mat_t *source, mat_t *target, double limit, bool *overLimit) {
    bool resultFlipped = false;
    long ctr = 0;
    mat_t *tmp;
    do {
        *overLimit = false;
        mat_smthr_t *smoother = mat_smthr_create_inner(source, target, limit, overLimit);
        mat_smthr_smooth(smoother);
        mat_smthr_destroy(smoother);

        tmp = target;
        target = source;
        source = tmp;
        resultFlipped = !resultFlipped;

        ctr++;
    } while (*overLimit);

    printf("%08li,", ctr);

    return resultFlipped ? source : target;
}

/* Used for type punning between a double and a unsigned long long int */
union {
    double d; // C99 double == 64 bytes
    unsigned long long int ll; // C99 long long >= 64 bytes
} double_ulld_punner_u;

/* Calculates a 64 byte (C99) parity of the given matrix */
unsigned long long int mat_parity(mat_t *matrix) {
    unsigned long long int currentParity = 0ULL;
    double *endPtr = matrix->data + matrix->ySize*matrix->ySize;
    for(double *tmpPtr = matrix->data; tmpPtr < endPtr; tmpPtr++) {
        double_ulld_punner_u.d = *tmpPtr;
        currentParity ^= double_ulld_punner_u.ll;
    }
    return currentParity;
}

/* Calculates a 64 byte crc of the given Matrix */
unsigned long long int mat_crc64(mat_t *matrix) {
    unsigned long long int currentCRC = 0ULL;
    double *endPtr = matrix->data + matrix->ySize*matrix->ySize;
    for(double *tmpPtr = matrix->data; tmpPtr < endPtr; tmpPtr++) {
        double_ulld_punner_u.d = *tmpPtr;
        currentCRC += double_ulld_punner_u.ll;
    }
    return currentCRC;
}

/* Copies the outer edge from source to target matrix
    If matrices of different size, behaviour undefined */
void mat_copy_edge(mat_t *source, mat_t *target) {
    mat_itr_edge_t *edgeIterator1 = mat_itr_edge_create(source);
    mat_itr_edge_t *edgeIterator2 = mat_itr_edge_create(target);
    while (mat_itr_edge_hasNext(edgeIterator1)) {
        *mat_itr_edge_next(edgeIterator2) = *mat_itr_edge_next(edgeIterator1);
    }
    mat_itr_edge_destroy(edgeIterator1);
    mat_itr_edge_destroy(edgeIterator2);
}

/* Pretty prints the matrix to STDOUT */
void mat_print(mat_t *matrix) {
    long size = matrix->xSize * matrix->ySize;
    double *ptr = matrix->data;
    for (long i = 1; i <= size; i++) {
        if (i % matrix->xSize != 0) {
            printf("%f, ", *ptr++);
        } else {
            printf("%f\n", *ptr++);
        }
    }
}

/* Compares the two matrices for equality */
bool mat_equals(mat_t *matrix1, mat_t *matrix2) {
    if (matrix1->ySize == matrix2->ySize && matrix1->xSize == matrix2->xSize) {
        bool match = true;
        mat_itr_t *matIterator1 = mat_itr_create_partial(matrix1, 0, 0, matrix1->xSize, matrix1->ySize);
        mat_itr_t *matIterator2 = mat_itr_create_partial(matrix2, 0, 0, matrix2->xSize, matrix2->ySize);
        while (mat_itr_hasNext(matIterator1) && match) {
            match = *mat_itr_next(matIterator1) == *mat_itr_next(matIterator2);
        }
        mat_itr_destroy(matIterator1);
        mat_itr_destroy(matIterator2);
        return match;
    } else {
        return false;
    }
}

/* Frees the given matrix from memory. Matrix must not be used afterwards. */
void mat_destroy(mat_t *matrix) {
    free(matrix->data);
    free(matrix);
}


