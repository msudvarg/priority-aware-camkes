#pragma once

#include <camkes.h>
#include <math.h>
#include "cycle_counter.h"

#define SAMPLES 100

//Defined in cycle_counter.h
#if defined CYCLE_COUNTER_ARM
    #define LOOPS 10000
#elif defined CYCLE_COUNTER_X86
    #define LOOPS 100000
#endif
       
void getstats(uint64_t * arr, int len) {

    double variance = 0;
    uint64_t min = 1000000000;
    uint64_t max = 0;
    uint64_t mean = 0;

    for (int i = 0; i < len; i++) {
        uint64_t x = arr[i];
        mean += x;
        if(x > max) max = x;
        if(x < min) min = x;
    }
    mean /= len;
    for (int i = 0; i < len; i++) {
        uint64_t diff = arr[i] - mean;
        variance += diff * diff;
    }
    variance = variance/(double)len;
    //printf("min %lu max %lu mean %lu stddev %f\n", min, max, mean, sqrt(variance));

//Defined in cycle_counter.h
#if defined CYCLE_COUNTER_ARM
    printf("%llu, %llu, %llu, %f\n", min, max, mean, sqrt(variance));
#elif defined CYCLE_COUNTER_X86
    printf("%lu, %lu, %lu, %f\n", min, max, mean, sqrt(variance));
#endif

}
