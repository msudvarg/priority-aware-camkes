#include <camkes.h>
#include "timing_headers/cycle_counter.h"

int run(void) {

    CYCLE_COUNTER_INIT();

    printf("Root dispatcher running ...\n");

    printf("Delay to allow all threads to initialize\n");    

    //Register a periodic timeout   
    int delay_ms = 2000; 
    seL4_CPtr notification = timeout_notification();
    seL4_Word badge;
    timeout_oneshot_relative(0, (delay_ms * NS_IN_MS));
    seL4_Wait(notification, &badge);

    printf("Dispatching 0\n");
    sd0_dispatch();
    printf("Dispatching 1\n");
    sd1_dispatch();
    printf("Dispatching 2\n");
    sd2_dispatch();
    printf("Dispatching 3\n");
    sd3_dispatch();
    printf("Dispatching 4\n");
    sd4_dispatch();
    
    
    printf("Done!\n");
}