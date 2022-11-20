/*

    priority-protocols.c

    The common implementation our concurrency framework for priority-aware intercomponent requests.
    Includes all functionality common to the supported protocols:
        Priority Propagation
        Non-Preemptive Critical Sections
        Immediate Priority Ceiling Protocol
        Priority Inheritance Protocol
    Additionally implements all protocols besides PIP

*/

#include "priority-protocols.h"
#include "priority-inheritance.h"

#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>


//Initialize a Priority_Protocol structure
void priority_protocol_init(struct Priority_Protocol * info,
        int priority_protocol, int priority) {

    //Only run on first thread
    if(!info->initialized) {
        info->initialized = true;
        info->priority_protocol = priority_protocol;
        info->priority_ceiling = priority;
    }
}

//Sets the caller's priority
void set_priority(int priority) {

#ifdef DEBUG
    printf("Setting priority to %d\n", priority);
#endif
    seL4_CPtr tcb = camkes_get_tls()->tcb_cap;
    int error = seL4_TCB_SetPriority(tcb,tcb,priority);
    ZF_LOGF_IFERR(error, "Failed to set priority to %d.\n", priority);
}

/*
    Pre and Post functions,
    which should run at the beginning and end of the interface handler function.
    These call different functions depending on the protocol used.
*/

void priority_pre(int request_priority, char * requestor, struct Priority_Protocol * info) {
    if (info->priority_protocol == propagated) {
        //Demote to request priority
        demote_priority(request_priority);
    }

    else if (info->priority_protocol == inherited) {
        //Enter priority inheritance
        priority_inheritance_enter(request_priority, requestor, info);
    }

    //Fixed priority is a no-op
    else {
        return;
    }

}

void priority_post(struct Priority_Protocol * info) {
    if (info->priority_protocol == propagated) {
        //Promote back to original HLP
        promote_priority(info->priority_ceiling);
    }

    else if (info->priority_protocol == inherited) {
        //Leave priority inheritance
        priority_inheritance_exit(info);
    }

    //Fixed priority is a no-op
    else {
        return;
    }
}

void nested_pre(void (*nest_fn)(int, const char*), struct Priority_Protocol * info) {
    if (info->priority_protocol == inherited) {
        //Promote to original HLP
        promote_priority(info->priority_ceiling);

        //Set function pointer to request's nest method
        info->pip->nest_fn = nest_fn;
    }

}

void nested_post(struct Priority_Protocol * info) {
    if(info->priority_protocol == inherited) {
        //Demote back to executing inherited priority
        demote_priority(info->pip->inherited_priority);
    }
}

void nest_rcv(int request_priority, const char * requestor, struct Priority_Protocol * info) {
    if(info->priority_protocol == inherited) {
        priority_inheritance_nest_rcv(request_priority, requestor, info->pip);
    }
}