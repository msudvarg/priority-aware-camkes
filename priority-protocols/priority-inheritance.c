/*

    priority-inheritance.c

    The implementation of Priority Inheritance Protocol

*/

#include "priority-protocols.h"
#include "notification-manager.h"

#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>


/*
    Initialize a Priority_Inheritance structure
    Initialization of the Notification_Manager is handled separately
*/
void priority_inheritance_init(struct Priority_Protocol * info,
        struct Priority_Inheritance * lock, unsigned num_threads) {

    //Only run on first thread
    if(!lock->initialized) {

        lock->initialized = true;

        //Set pointer to function-scope static Priority_Inheritance object
        info->pip = lock;

        //Initialize fields of Priority_Inheritance object
        lock->locked = false;
        lock->num_threads = num_threads;
        lock->nest_fn = NULL;

#ifdef DEBUG
        printf("(0) PIP initialized with %d threads at priority %d\n", num_threads, info->priority_ceiling);
#endif

    }
}

void inherit(struct Priority_Inheritance * lock, int request_priority) {

    //Allow running thread to inherit waiter's priority
    //(2)
    if(request_priority > lock->inherited_priority) {

#ifdef DEBUG
        printf("(3) PIP: elevating priority from %d to %d\n", lock->runner_tcb, request_priority);
#endif
        lock->inherited_priority = request_priority; //(3)

        //(4) Set inherited priority
        int error = seL4_TCB_SetPriority(lock->runner_tcb, lock->runner_tcb, request_priority);
        ZF_LOGF_IFERR(error, "Failed to set runner's priority to %d.\n", request_priority);

        //(5) If a request is pending, forward elevated priority to request endpoint
        if (lock->nest_fn) {
#ifdef DEBUG
            printf("(5) PIP: forwarding new priority %d to downstream\n", request_priority);
#endif
            lock->nest_fn(request_priority, lock->requestor);
        }
    }
}


void priority_inheritance_nest_rcv(int request_priority, int requestor, struct Priority_Inheritance * lock) {

    //(26) Not lockholder, search ntfn mgr
#ifdef DEBUG
    printf("(26) PIP: updating waiting thread %d to priority %d\n", requestor, request_priority);
#endif
    if (lock->requestor != requestor) {
        ntfn_mgr_update(request_priority, requestor, &lock->ntfn_mgr);
    }

    //(2)-(5)
    inherit(lock, request_priority);
}

/*
    priority_inheritance_enter
    
    Begins the Priority Inheritance Protocol,
    should run before the endpoint handler code.
*/
void priority_inheritance_enter(int request_priority, int requestor, struct Priority_Protocol * info) {

    struct Priority_Inheritance * lock = info->pip;

    //Check if we can obtain the lock.
    //No while loop needed: when we wake up, lock is guaranteed to be available.
    //(1)
    if(lock->locked) {
        //The lock is locked

        //(2)-(5)
        inherit(lock, request_priority);

        //(6)-(8) Wait on a notification object
#ifdef DEBUG
        printf("(6) PIP: thread %d entering ntfn mgr with priority %d\n", requestor, request_priority);
#endif
        ntfn_mgr_wait(&request_priority, requestor, &lock->ntfn_mgr);
#ifdef DEBUG
        printf("(8) PIP: thread %d leaving ntfn mgr with priority %d\n", requestor, request_priority);
#endif

    }

    //(9) We obtain the lock
    lock->locked = true;

    //Set the inherited priority and TCB to our parameters
    //Note: request_priority is appropriately updated from upstream nest
    //even if the thread was waiting in ntfn mgr.
    //ntfn mgr holds a pointer to the variable on the stack,
    //and the stack frame persists across the call and return from wait
    lock->inherited_priority = request_priority; //(10)
    lock->runner_tcb = camkes_get_tls()->tcb_cap; //(11)
    lock->requestor = requestor; //(12)
    lock->nest_fn = NULL; //(13)

#ifdef DEBUG
        printf("(14) PIP: thread %d demoting to priority %d to run\n", requestor, request_priority);
#endif
    //(14) Demote priority to run request code
    demote_priority(request_priority);

    //(15) Component-defined interface function now runs
}


/*
    priority_inheritance_exit
    
    Ends the Priority Inheritance Protocol,
    should run after the endpoint handler code,
    before it returns a value and waits on the endpoint.
*/
void priority_inheritance_exit(struct Priority_Protocol * info) {

    //(22) Promote priority
    promote_priority(info->priority_ceiling);
#ifdef DEBUG
        printf("(22) PIP: thread promoted to priority %d to reply\n", info->priority_ceiling);
#endif

    //(23) Mark unlocked
    info->pip->locked = false;

    //(24) Signal waiters
    ntfn_mgr_signal(&info->pip->ntfn_mgr);

    //(25) Reply and wait implicit after function return
}

void priority_inheritance_nested_pre(int * msg_priority,
        void (*nest_fn)(const int, const int), struct Priority_Protocol * info) {

    //(16) Promote to original HLP
    promote_priority(info->priority_ceiling);

    //(17) Set message priority to current inherited priority
    *msg_priority = info->pip->inherited_priority;
#ifdef DEBUG
        printf("(17) PIP: thread setting priority for nested downstream request to %d\n", *msg_priority);
#endif

    //(18) Set function pointer to request's nest method
    info->pip->nest_fn = nest_fn;

    //(19) Nested request function now runs

}

void priority_inheritance_nested_post(struct Priority_Protocol * info) {

    //(20) Clear nest function
    info->pip->nest_fn = NULL;

    //(21) Demote back to executing inherited priority
#ifdef DEBUG
        printf("(21) PIP: thread demoting to priority %d after nest\n", info->pip->inherited_priority);
#endif
    demote_priority(info->pip->inherited_priority);
    
}