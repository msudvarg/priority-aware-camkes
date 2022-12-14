/*

    service-forwarder.c

    Implements functionality for a ServiceForwarder component.
    A simple example demonstrating nested component requests.
    Provides a power function, which is implemented by
    forwarding the request to another component.

*/

#include <camkes.h>

#include "nest.h"
NEST(l1)
NEST(l2)

void l1_init(void) {

}

int l1_pow(const int base, const int exp, int priority, const int requestor) {
    printf("Received request from %d with priority %d\n", requestor, priority);
    return REQUEST(l1, r_nest, pow, base, exp);
}

void l2_init(void) {

}

int l2_pow(const int base, const int exp, int priority, const int requestor) {
    printf("Received request from %d with priority %d\n", requestor, priority);
    return REQUEST(l1, r_nest, pow, base, exp);
}
