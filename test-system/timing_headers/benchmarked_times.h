#pragma once

#include "cycle_counter.h"

/* Hardware-tested constants */

//Defined in cycle_counter.h
#if defined CYCLE_COUNTER_ARM

    /* Raspberry Pi */

    //Execution time is in microseconds
    //Max: 849009 cycles for 10000 loops
    //10000 loops / 849009 cycles * 700 cycles / us = 8.2449 loops per us
    const static float loops_per_us = 8.2449;

    const static int cycle_get = 9;
    const static int standard_request = 3168;
    const static int standard_reply = 1625;
    const static int pp_request = 5740;
    const static int pp_reply = 2191;
    const static int fixed_request = 2691;
    const static int fixed_reply = 1137;
    const static int inherited_request = 5943;
    const static int inherited_reply = 2555;
    const static int heap_3_time = 476;
    const static int cpu_speed = 700;
    const static int dispatch_time = 304;
    const static int dispatch_overhead = standard_request + standard_reply + dispatch_time + 2*cycle_get;


//Defined in cycle_counter.h
#elif defined CYCLE_COUNTER_X86

    /* Intel x64 */

    //Execution time is in microseconds
    //Max: 967448 cycles for 100000 loops
    //100000 loops / 967448 cycles * 2100 cycles / us = 217.066 loops per us
    const static float loops_per_us = 10;// 217.066;

    const static int cycle_get = 52;
    const static int standard_request = 3344;
    const static int standard_reply = 2266;
    const static int pp_request = 6870;
    const static int pp_reply = 4464;
    const static int fixed_request = 3664;
    const static int fixed_reply = 2888;
    const static int inherited_request = 6286;
    const static int inherited_reply = 4258;
    const static int heap_3_time = 532;
    const static int cpu_speed = 2100;
    const static int dispatch_time = 1638;
    const static int dispatch_overhead = standard_request + standard_reply + dispatch_time + 2*cycle_get;

#endif