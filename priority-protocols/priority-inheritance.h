/*

    priority-inheritance.h

    The public interface for the implementation of Priority Inheritance Protocol.

*/

#pragma once

#include "notification-manager.h"

#include <camkes.h>
#include <sel4/sel4.h>

struct Priority_Inheritance {
    bool locked;
    bool initialized;
    seL4_Word inherited_priority;
    seL4_CPtr runner_tcb;
    struct Notification_Manager ntfn_mgr;
    unsigned num_threads;
};

/*
    Priority Inheritance Init

    Allocates a static Priority_Inheritance object.
    Even though it's in the init function scope,
    we access it through the pointer in the Priority_Protocol object.

    Calls the priority_inheritance_init function to initialize
    the Priority_Inheritance object.
*/
#define PRIORITY_INHERITANCE_INIT(PRIORITY_PROTOCOL_PTR, NUM_THREADS, PRIORITY) \
    static struct Priority_Inheritance lock; \
    priority_inheritance_init(PRIORITY_PROTOCOL_PTR, \
            &lock, NUM_THREADS);

void priority_inheritance_init(struct Priority_Protocol * info,
        struct Priority_Inheritance * lock, unsigned num_threads);

/*
    Enter and Exit functions,
    which should run at the beginning and end of the interface handler function,
    called from priority_pre and priority_post functions of priority-protocols.h
    if Priority Inheritance Protocol is being used.
*/
void priority_inheritance_enter(int priority, struct Priority_Protocol * info);

void priority_inheritance_exit(struct Priority_Protocol * info);