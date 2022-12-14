#define CYCLE_COUNTER_ARM

#if defined CYCLE_COUNTER_ARM
    #include "perfmon.h"
    #define CYCLE_COUNTER_INIT() pmccntr_enable_once(1, 0)
    #define CYCLE_COUNTER_GET() pmccntr_get()
#elif defined CYCLE_COUNTER_X86
    #include "rdtsc.h"
    #define CYCLE_COUNTER_INIT()
    #define CYCLE_COUNTER_GET() rdtsc_get()
#endif