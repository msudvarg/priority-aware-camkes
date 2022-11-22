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
        printf("initialized priority inheritance lock with %d threads\n", num_threads);
#endif

    }
}

void inherit(struct Priority_Inheritance * lock, int request_priority) {

    //Allow running thread to inherit waiter's priority
    //(2)
    if(request_priority > lock->inherited_priority) {

        lock->inherited_priority = request_priority; //(3)
#ifdef DEBUG
        printf("Setting priority of runner TCB %d\n", lock->runner_tcb);
#endif

        //(4)
        int error = seL4_TCB_SetPriority(lock->runner_tcb, lock->runner_tcb, request_priority);
        ZF_LOGF_IFERR(error, "Failed to set runner's priority to %d.\n", request_priority);

        //If a request is pending, forward elevated priority to request endpoint
        if (lock->nest_fn) lock->nest_fn(request_priority, lock->requestor);
    }
}


void priority_inheritance_nest_rcv(int request_priority, int requestor, struct Priority_Inheritance * lock) {

    //Not lockholder, search ntfn mgr
    if (lock->requestor != requestor) {
        ntfn_mgr_update(request_priority, requestor, &lock->ntfn_mgr);
    }

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

        inherit(lock, request_priority);

        //Wait on a notification object
        ntfn_mgr_wait(&request_priority, requestor, &lock->ntfn_mgr);

    }

    //We obtain the lock
    lock->locked = true;

    //Set the inherited priority and TCB to our parameters
    lock->inherited_priority = request_priority;
    lock->runner_tcb = camkes_get_tls()->tcb_cap;
    lock->requestor = requestor;

    //Demote priority to run request code
    demote_priority(request_priority);

    //Component-defined interface function now runs
}


/*
    priority_inheritance_exit
    
    Ends the Priority Inheritance Protocol,
    should run after the endpoint handler code,
    before it returns a value and waits on the endpoint.
*/
void priority_inheritance_exit(struct Priority_Protocol * info) {

    //Promote priority
    promote_priority(info->priority_ceiling);

    //Mark unlocked
    info->pip->locked = false;

    //Signal waiters
    ntfn_mgr_signal(&info->pip->ntfn_mgr);

    //Reply and wait implicit after function return
}

void priority_inheritance_nested_pre(int * msg_priority,
        void (*nest_fn)(const int, const int), struct Priority_Protocol * info) {

    //Promote to original HLP
    promote_priority(info->priority_ceiling);

    //Set message priority to current inherited priority
    *msg_priority = info->pip->inherited_priority;

    //Set function pointer to request's nest method
    info->pip->nest_fn = nest_fn;

}

void priority_inheritance_nested_post(struct Priority_Protocol * info) {

    //Clear nest function
    info->pip->nest_fn = NULL;

    //Demote back to executing inherited priority
    demote_priority(info->pip->inherited_priority);
    
}