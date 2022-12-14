#pragma once

static inline uint64_t rdtsc_get(void) {
    uint32_t high, low;
    //asm volatile ("lfence");
    asm volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ( (uint64_t) low) |
            ( ( (uint64_t) high ) << 32llu );
}   