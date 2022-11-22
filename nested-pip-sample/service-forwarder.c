/*

    service-forwarder.c

    Implements functionality for a ServiceForwarder component.
    A simple example demonstrating nested component requests.
    Provides a power function, which is implemented by
    forwarding the request to another component.

*/

#include <camkes.h>

#include "nest.h"
NEST(r)

void r_init(void) {

}

int r_pow(const int base, const int exp, const int priority, const int requestor) {
    return r_nest_pow(base, exp, priority, requestor);
}
