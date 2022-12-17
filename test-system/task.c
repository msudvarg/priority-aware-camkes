/*

    task.c

*/

#include <camkes.h>
#include "timing_headers/spin.h"

static int misses;

void rd_dispatch(void) {
    spin(execution_time_pre);
    r_request(_priority, requestor);
    spin(execution_time_post);
    //printf("Task %d done\n", requestor);
}