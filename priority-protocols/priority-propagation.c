/*

    priority-propagation.c

    The implementation of Priority Propagation

*/

#include "priority-protocols.h"

#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>


/*
    Initialize a Priority_Propagation structure
*/
void priority_propagation_init(struct Priority_Protocol * info,
        struct Priority_Propagation * prop, struct Propagated_Thread * threads,
        unsigned num_threads) {

    //Only run on first thread
    if(!prop->initialized) {

        prop->initialized = true;

        //Set pointer to function-scope static Priority_Propagation object
        info->prop = prop;

        //Initialize fields of Priority_Propagation object
        prop->active_list = NULL;
        prop->free_list = threads;
        prop->arr_size = num_threads;
        prop->thread_arr = threads;

        //Initialize linked list
        for (int i = 0; i < num_threads - 1; ++i) {
            threads[i].next = &threads[i+1];
        }
        threads[num_threads - 1].next = NULL;

#ifdef DEBUG
        printf("initialized priority propagation info with %d threads\n", num_threads);
#endif

    }
}

void priority_propagation_enter(int request_priority, int requestor, struct Priority_Protocol * info) {

    struct Priority_Propagation * prop = info->prop;

    //(1) Grab node from free list
    struct Propagated_Thread * thread = prop->free_list;
    prop->free_list = thread->next;
    thread->next = prop->active_list;
    prop->active_list = thread;

    //Set its info
    thread->priority = request_priority; //(2)
    thread->runner_tcb = camkes_get_tls()->tcb_cap; //(3)
    thread->nest_fn = NULL; //(4)
    thread->requestor = requestor; //(5)

    //(6) Demote to request priority
    demote_priority(request_priority);

    //(7) Component-defined interface function now runs

}

void priority_propagation_exit(int requestor, struct Priority_Protocol * info) {

    //(14) Promote back to original HLP
    promote_priority(info->priority_ceiling);
    
    struct Priority_Propagation * prop = info->prop;

    //Find node
    struct Propagated_Thread * thread = prop->active_list;
    struct Propagated_Thread * back = NULL;
    while (thread->requestor != requestor) {
        back = thread;
        thread = thread->next;
    }

    //(15) Clear node
    thread->runner_tcb = 0;

    //Return to free list
    if(back) back->next = thread->next;
    else prop->active_list = thread->next;
    thread->next = prop->free_list;
    prop->free_list = thread;

    //(16) Reply and wait implicit after function return

}


void priority_propagation_nest_rcv(int request_priority, int requestor, struct Priority_Propagation * prop) {

    //(17) Find node
    struct Propagated_Thread * thread = prop->active_list;
    while (thread->requestor != requestor) {
        thread = thread->next;
    }

    //(18) Set priority
    thread->priority = request_priority;
    int error = seL4_TCB_SetPriority(thread->runner_tcb, thread->runner_tcb, request_priority);
    ZF_LOGF_IFERR(error, "Failed to set runner's priority to %d.\n", request_priority);

    //(19) If a request is pending, forward elevated priority to request endpoint
    if (thread->nest_fn) thread->nest_fn(request_priority, requestor);

}


void priority_propagation_nested_pre(int * msg_priority, int requestor,
        void (*nest_fn)(const int, const int), struct Priority_Protocol * info) {

    //(8) Promote to original HLP
    promote_priority(info->priority_ceiling);

    //Find node
    struct Propagated_Thread * thread = info->prop->active_list;
    while (thread->requestor != requestor) {
        thread = thread->next;
    }    

    //(9) Set message priority to current inherited priority
    *msg_priority = thread->priority;

    //(10) Set function pointer
    thread->nest_fn = nest_fn;

    //(11) Nested request function now runs

}

void priority_propagation_nested_post(int requestor, struct Priority_Protocol * info) {

    //Find node
    struct Propagated_Thread * thread = info->prop->active_list;
    while (thread->requestor != requestor) {
        thread = thread->next;
    }

    //(12) Clear nest function
    thread->nest_fn = NULL;

    //(13) Demote back to executing inherited priority
    demote_priority(thread->priority);

}