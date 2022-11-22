/*

    service-terminator.c

    Implements functionality for a ServiceTerminator component.
    A simple example demonstrating a component terminating a request chain.
    Provides a power function, which raises an integer base to an integer exponent,
    both provided as request parameters,
    then replies with the result.

*/

#include <camkes.h>

void r_init(void) {

}

int r_pow(const int base, int exp, const int priority, const int requestor) {
    int res = 1;
    while (exp > 0) {
        res *= base;
        exp--;
    }
    return res;
}