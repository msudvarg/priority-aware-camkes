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

int r_pow(const int base, const int exp, int priority, const int requestor) {
    printf("Received request from %d with priority %d\n", requestor, priority);
    return REQUEST(r, r_nest, pow, base, exp);
}
