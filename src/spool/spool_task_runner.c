//
// Created by dominic on 12/12/16.
//

#include <stdlib.h>
#include <pthread.h>
#include "spool_task_runner.h"

typedef struct SPOOL_ARGS_T {
    spool_task_t* spool_task;
    spool_t* spool;
} spool_args_t;

typedef struct SPOOL_T {
    unsigned int id;
    pthread_t* thread;
    spool_args_t* args;
} spool_t;

spool_t *spool_init(spool_t* spool, spool_task_t *spool_task, unsigned int id) {
    spool_t *dispatcher = malloc(sizeof(spool_t));
    dispatcher->args = malloc(sizeof(spool_args_t));
    dispatcher->args->spool_task = spool_task;
    dispatcher->args->spool = spool;
    dispatcher->thread = malloc(sizeof(pthread_t));
    dispatcher->id = id;
    return dispatcher;
}

void *spool_run_inner(void *args) {
    spool_args_t *spool_args = args;
    spool_task_run(spool_args->spool_task, spool_args->spool);
    // TODO Implement callback to inform of completion.
    return NULL;
}

unsigned int spool_get_id(spool_t *dispatcher) {
    return dispatcher->id;
}

void spool_run(spool_t *dispatcher) {
    pthread_create(dispatcher->thread, NULL, spool_run_inner, dispatcher->args);
}

void spool_destroy(spool_t *dispatcher) {
    free(dispatcher->args);
    free(dispatcher->thread);
    free(dispatcher);
}