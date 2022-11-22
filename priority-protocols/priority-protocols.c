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
#include "priority-propagation.h"

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

void priority_pre(int request_priority, int requestor, struct Priority_Protocol * info) {
    if (info->priority_protocol == propagated) {
        //Enter priority propagation
        priority_propagation_enter(request_priority, requestor, info);
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

void priority_post(int requestor, struct Priority_Protocol * info) {
    if (info->priority_protocol == propagated) {
        //Leave priority propagation
        priority_propagation_exit(requestor, info);
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

void nested_pre(int requestor,
        void (*nest_fn)(const int, const int), struct Priority_Protocol * info) {

    if (info->priority_protocol == inherited) {
        priority_inheritance_nested_pre(nest_fn, info);
        return;
    }

    if (info->priority_protocol == propagated) {
        priority_propagation_nested_pre(requestor, nest_fn, info);
        return;
    }

}

void nested_post(int requestor, struct Priority_Protocol * info) {

    if(info->priority_protocol == inherited) {
        priority_inheritance_nested_post(info);
        return;
    }

    if(info->priority_protocol == propagated) {
        priority_propagation_nested_post(requestor, info);
    }
}

void nest_rcv(int request_priority, int requestor, struct Priority_Protocol * info) {
    if(info->priority_protocol == inherited) {
        priority_inheritance_nest_rcv(request_priority, requestor, info->pip);
    }
    if(info->priority_protocol == propagated) {
        priority_propagation_nest_rcv(request_priority, requestor, info->prop);
    }
}