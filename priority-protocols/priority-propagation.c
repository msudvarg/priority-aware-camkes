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
        printf("(0) Prop initialized with %d threads at priority %d in %s\n", num_threads, info->priority_ceiling, get_instance_name());
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

#ifdef DEBUG
        printf("(6) Prop: thread %d demoting to priority %d to run in %s\n", requestor, request_priority, get_instance_name());
#endif
    //(6) Demote to request priority
    demote_priority(request_priority);

    //(7) Component-defined interface function now runs

}

void priority_propagation_exit(int requestor, struct Priority_Protocol * info) {

    //(14) Promote back to original HLP
    promote_priority(info->priority_ceiling);
#ifdef DEBUG
        printf("(14) Prop: thread promoted to priority %d to reply in %s\n", info->priority_ceiling, get_instance_name());
#endif
    
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
#ifdef DEBUG
    printf("(18) Prop: updating thread %d to priority %d in %s\n", requestor, request_priority, get_instance_name());
#endif
    thread->priority = request_priority;
    int error = seL4_TCB_SetPriority(thread->runner_tcb, thread->runner_tcb, request_priority);
    ZF_LOGF_IFERR(error, "Failed to set runner's priority to %d.\n", request_priority);

    //(19) If a request is pending, forward elevated priority to request endpoint
#ifdef DEBUG
    printf("(19) Prop: forwarding new priority %d to downstream in %s\n", request_priority, get_instance_name());
#endif
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
#ifdef DEBUG
        printf("(9) Prop: thread setting priority for nested downstream request to %d in %s\n", *msg_priority, get_instance_name());
#endif

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
#ifdef DEBUG
        printf("(13) Prop: thread demoting to priority %d after nest in %s\n", thread->priority, get_instance_name());
#endif
    demote_priority(thread->priority);

}