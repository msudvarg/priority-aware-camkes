#include <camkes.h>
#include "timing_headers/spin.h"

#include "nest.h"
NEST(r1)
NEST(r2)

void r1_init(void) {}
void r2_init(void) {}

long r1_request(int priority, const int requestor) {
    
    spin(r1_execution_time);
    REQUEST(r1, r, request);
    spin(r1_execution_time_post);
    
}

long r2_request(int priority, const int requestor) {
    
    spin(r2_execution_time);
    REQUEST(r2, r, request);
    spin(r2_execution_time_post);

}

