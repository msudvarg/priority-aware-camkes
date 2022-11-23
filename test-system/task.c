/*

    task.c

    Registers a periodic timeout with a CAmkES TimeServer global component.
    At each release, increments a global iterations variable,
    then prints the result of raising task priority to that number of iterations.
    The power is implemented as a request to a ServiceForwarder component.

*/

#include <camkes.h>


int release_count = 0;

void task(void) {

    printf("Task %s: %d^%d=%d\n",
        get_instance_name(), _priority,
        release_count,
        r_pow(_priority, release_count, _priority, requestor));

    release_count++;

}

int run(void) {

    //Register a periodic timeout    
    seL4_CPtr notification = timeout_notification();
    seL4_Word badge;
    timeout_periodic(0, (period_ms * NS_IN_MS));

    //Execute task procedure after every release
    while(1) {
        task();
        seL4_Wait(notification, &badge);
    }

}