#include <camkes.h>
#include "timing_headers/spin.h"

#include "nest.h"
NEST(r)

void r_init(void) {}

long r_request(int priority, const int requestor) {
    
    spin(r_execution_time);
    REQUEST(r, r1, request);
    REQUEST(r, r2, request);
    spin(r_execution_time_post);
    
}

