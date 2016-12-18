//
// Created by dominic on 13/12/16.
//

#include "dispatcher.h"

typedef struct DISPATCHER_T {

} dispatcher_t ;

dispatcher_t *dispatcher_init() {

}

void dispatcher_run(dispatcher_t* dispatcher) {
    // Take args for run including original matrix and requested precision.
    // Create semaphore to wait for all replies.
    // Split matrix into sections for smoothing and send each section to appropriate node.
}

void dispatcher_finished(dispatcher_t* dispatcher) {
    // Decrement sem.
}

void dispatcher_destroy(dispatcher_t* dispatcher) {
    //FIXME Implement!
}