/*

    priority-protocols.h

    The common header implementing our concurrency framework for priority-aware intercomponent requests.
    
    Motivation, design, implementation, and evaluation are described in the following paper:

    M. Sudvarg and C. Gill. A Concurrency Framework for Priority-Aware Intercomponent Requests in CAmkES on seL4.
    The 28th IEEE International Conference on Embedded and Real-Time Computing Systems and Applications (RTCSA),
    August 2022.
    Available from https://www.sudvarg.com/priority-aware-camkes/
*/

#pragma once

#include <camkes.h>

//These are the priority protocols we support
enum priority_protocols {
    propagated,
    inherited,
    fixed
};

struct Priority_Protocol {
    bool initialized;
    int priority_protocol;
    int priority_ceiling;
    struct Priority_Inheritance * pip;
};

#include "priority-inheritance.h"

//Initialize a Priority_Protocol structure
void priority_protocol_init(struct Priority_Protocol * info,
        int priority_protocol, int priority);


/*
    Note that currently, promote_priority and demote_priority
    are just wrappers around set_priority.
    Functionally, they are equivalent.
    However, aspectually, they are different,
    and so we maintain separate functions in case we need different functionality.
*/

//Sets the caller's priority
void set_priority(int priority);

//Demotes the caller's priority
static inline void demote_priority(int priority) {
    set_priority(priority);
}

//Promotes the caller's priority
static inline void promote_priority(int original_priority) {
    set_priority(original_priority);
}


/*
    Pre and Post functions,
    which should run at the beginning and end of the interface handler function.
    These call different functions depending on the protocol used.
*/
void priority_pre(int request_priority, int requestor, struct Priority_Protocol * info);
void priority_post(struct Priority_Protocol * info);

/*
    Pre and Post functions,
    which should run before and after a request is made along a nested PIP path.
*/
void nested_pre(void (*nest_fn)(const int, const int), struct Priority_Protocol * info);
void nested_post(struct Priority_Protocol * info);

void nest_rcv(int request_priority, int requestor, struct Priority_Protocol * info);
