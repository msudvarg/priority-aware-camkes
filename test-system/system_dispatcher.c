#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>
#include "miss_data.h"

//Sets the caller's priority
void set_priority(int priority) {
    seL4_CPtr tcb = camkes_get_tls()->tcb_cap;
    int error = seL4_TCB_SetPriority(tcb,tcb,priority);
    ZF_LOGF_IFERR(error, "Failed to set priority to %d.\n", priority);
}

void sd_dispatch() {


    //Top priority
    td1_emit();
    td2_emit();
    td3_emit();

    set_priority(0);

    //Lowest priority
    tdc1_wait();
    tdc2_wait();
    tdc3_wait();

    struct Miss_Data * md = (struct Miss_Data *)miss_data;
    for (int i = 0; i < NUM_TASKS; i++) {
        int misses = md[i].misses;
        int hits = md[i].periods - misses;
        printf("Task %d %d %d\n", i+1, misses, hits);

    }
}