/*

    priority-propagation.h

    The public interface for the implementation of Priority Propagation

*/

#pragma once

#include <camkes.h>
#include <sel4/sel4.h>

struct Propagated_Thread {
    seL4_CPtr runner_tcb;
    int requestor;
    seL4_Word priority;
    void (*nest_fn)(const int, const int);
    struct Propagated_Thread * next;
};

struct Priority_Propagation {

    bool initialized;

    //Array of Propagated Threads, passed at initialization
    struct Propagated_Thread * thread_arr;

    //We implement as two linked lists:
    //the Active list and the Free list
    struct Propagated_Thread * active_list;
    struct Propagated_Thread * free_list;

    unsigned arr_size;
}

#define PRIORITY_PROPAGATION_INIT(PRIORITY_PROTOCOL_PTR, NUM_THREADS) \
    static struct Priority_Propagation prop; \
    static struct Propagated_Thread threads[ARR_SIZE]; \
    priority_propagation_init(PRIORITY_PROTOCOL_PTR, &prop, threads, NUM_THREADS);

void priority_propagation_init(struct Priority_Protocol * info,
        struct Priority_Propagation * prop, struct Propagated_Thread * threads,
        unsigned num_threads);

void priority_propagation_enter(int request_priority, int requestor, struct Priority_Protocol * info);
void priority_propagation_exit(int requestor, struct Priority_Protocol * info);

void priority_propagation_nest_rcv(int request_priority, int requestor, struct Priority_Propagation * prop);

void priority_propagation_nested_pre(int requestor,
        void (*nest_fn)(const int, const int), struct Priority_Protocol * info);
void priority_propagation_nested_post(int requestor, struct Priority_Protocol * info);