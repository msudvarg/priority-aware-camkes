#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>
#include "timing_headers/spin.h"
#include "timing_headers/cycle_counter.h"
#include "miss_data.h"

//Sets the caller's priority
void set_priority(int priority) {
    seL4_CPtr tcb = camkes_get_tls()->tcb_cap;
    int error = seL4_TCB_SetPriority(tcb,tcb,priority);
    ZF_LOGF_IFERR(error, "Failed to set priority to %d.\n", priority);
}

static int misses;

int run(void) {
    //printf("Instance %s running\n", get_instance_name());

    td_wait();

    if(!periods || !period) {
        r_dispatch();
        return 0;
    }


    seL4_CPtr notification = timeout_notification();
    seL4_Word badge;

    timeout_periodic(0, (period * NS_IN_US));

    //set_priority(priority - 10); //num_harmonic_periods * 2

    uint64_t start, end;
    
    for(int i = 0; i < periods; i++) {
        start = CYCLE_COUNTER_GET();
        r_dispatch();
        end = CYCLE_COUNTER_GET();
        if ( (end - start + dispatch_time)/cpu_speed > period ) misses++; //Elapsed time greater than period
        seL4_Wait(notification, &badge);
    }

    timeout_stop(0);

    struct Miss_Data * md = (struct Miss_Data *)miss_data;
    md[task_id - 1].misses = misses;
    md[task_id - 1].periods = periods;

    tdc_emit();

}