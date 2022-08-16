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

#ifdef DEBUG
        printf("initialized priority inheritance lock with %d threads\n", num_threads);
#endif

    }
}

/*
    priority_inheritance_enter
    
    Begins the Priority Inheritance Protocol,
    should run before the endpoint handler code.
*/
void priority_inheritance_enter(int request_priority, struct Priority_Protocol * info) {

    struct Priority_Inheritance * lock = info->pip;

    //Check if we can obtain the lock.
    //No while loop needed: when we wake up, lock is guaranteed to be available.
    if(lock->locked) {
        //The lock is locked

        //Allow running thread to inherit waiter's priority
        if(request_priority > lock->inherited_priority) {
            lock->inherited_priority = request_priority;
#ifdef DEBUG
            printf("Setting priority of runner TCB %d\n", lock->runner_tcb);
#endif
            int error = seL4_TCB_SetPriority(lock->runner_tcb, lock->runner_tcb, request_priority);
            ZF_LOGF_IFERR(error, "Failed to set runner's priority to %d.\n", request_priority);
        }

        //Wait on a notification object
        ntfn_mgr_wait(request_priority, &lock->ntfn_mgr);

    }

    //We obtain the lock
    lock->locked = true;

    //Set the inherited priority and TCB to our parameters
    lock->inherited_priority = request_priority;
    lock->runner_tcb = camkes_get_tls()->tcb_cap;

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
