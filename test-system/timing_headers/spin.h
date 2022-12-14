#pragma once

#include <camkes.h>
#include <camkes/tls.h>
#include <sel4utils/sel4_zf_logif.h>
#include "benchmark.h"
#include "benchmarked_times.h"



void spin(int execution_time) {
    
    float loops_f = (float) execution_time * loops_per_us;
    int loops = loops_f; //Round down
    //printf("Executing %d loops to get %d microseconds\n", loops, execution_time);
    float x = 0;
    volatile float y;
    for (int j = 0; j < loops; j++) {
        x += (float)(j % 11 - 5) * 3.141592653F;
    }
    y = x;
    
   return;
}
