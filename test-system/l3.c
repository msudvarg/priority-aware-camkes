#include <camkes.h>
#include "timing_headers/spin.h"

#include "nest.h"
NEST(r)

void r_init(void) {}

long r_request(const int priority, const int requestor) {
    
    spin(r_execution_time);
    
}

