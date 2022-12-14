
#ifndef __ASMARM_ARCH_PERFMON_H
#define __ASMARM_ARCH_PERFMON_H

/******************************************************************************
* 
* perfmon.h
*
* Provides functions and constant flags
* for interracting with the ARM Cortex-A53
* performance monitors in AARCH32 mode.
*
* This is the architecture in use on the Raspberry Pi 3 Model B series.
*
* Written October 9, 2020 by Marion Sudvarg
* 
******************************************************************************/

//PMCR: Performance Monitor Control Register

#define PMCR_ENABLE_COUNTERS (1 << 0)
#define PMCR_EVENT_COUNTER_RESET (1 << 1)
#define PMCR_CYCLE_COUNTER_RESET (1 << 2)
#define PMCR_CYCLE_COUNT_64 (1 << 3) //Increment cycle count every 64 cycles
#define PMCR_EXPORT_ENABLE (1 << 4)
#define PMCR_CYCLE_COUNTER_DISABLE (1 << 5)
#define PMCR_CYCLE_COUNTER_64_BIT (1 << 6) //Cycle counter overflows at 64 bits instead of 32


const unsigned PMCR_WRITE = PMCR_ENABLE_COUNTERS | PMCR_EVENT_COUNTER_RESET |  PMCR_CYCLE_COUNTER_RESET | PMCR_CYCLE_COUNT_64 | PMCR_EXPORT_ENABLE | PMCR_CYCLE_COUNTER_DISABLE | PMCR_CYCLE_COUNTER_64_BIT;
const unsigned PMCR_READ = PMCR_ENABLE_COUNTERS | PMCR_CYCLE_COUNT_64 | PMCR_EXPORT_ENABLE | PMCR_CYCLE_COUNTER_DISABLE | PMCR_CYCLE_COUNTER_64_BIT;

static inline void pmcr_set(unsigned x) {
	asm volatile ("MCR p15, 0, %0, c9, c12, 0\t\n" :: "r" (x));
}

static inline char pmcr_set_check(unsigned x) {
	if ((x & PMCR_WRITE) == x) {
		pmcr_set(x);
		return 1;
	}
	return 0;
}

static inline unsigned long pmcr_get(void) {
	unsigned long x = 0;
	asm volatile ("MRC p15, 0, %0, c9, c12, 0\t\n" : "=r" (x));
	return x;
}

//CNTENS: Count Enable Set Register

const unsigned CNTENS_CTR0 = 1 << 0;
const unsigned CNTENS_CTR1 = 1 << 1;
const unsigned CNTENS_CTR2 = 1 << 2;
const unsigned CNTENS_CTR3 = 1 << 3;
const unsigned CNTENS_CYCLE_CTR = 1 << 31;

static inline void cntens_set(unsigned x) {
	asm volatile ("MCR p15, 0, %0, c9, c12, 1\t\n" :: "r" (x));
}

static inline unsigned long cntens_get(void) {
	unsigned long x = 0;
	asm volatile ("MRC p15, 0, %0, c9, c12, 1\t\n" : "=r" (x));
	return x;
}

//PMCCNTR: Performance Monitors Cycle Count Register

static inline unsigned long long pmccntr_get(void) {
	/*
	register unsigned long pmcr = pmcr_get();
	if(pmcr & PMCR_CYCLE_COUNTER_64_BIT) {
		unsigned long low, high;
		asm volatile ("MRRC p15, 0, %0, %1, c9" : "=r" (low), "=r" (high));
		return ( (unsigned long long) low) |
				( ( (unsigned long long) high ) << 32 );

	}
	else {
		unsigned long cycle_count;
		asm volatile ("MRC p15, 0, %0, c9, c13, 0" : "=r" (cycle_count));
		return (unsigned long long) cycle_count;
	}
	*/
	unsigned long low, high;
	asm volatile ("MRRC p15, 0, %0, %1, c9" : "=r" (low), "=r" (high));
	return ( (unsigned long long) low) |
			( ( (unsigned long long) high ) << 32 );
}

static inline void pmcr_enable_pmccntr(char cycles_64_bit, char count_every_64) {
	unsigned long flags = pmcr_get();
	flags = flags | PMCR_ENABLE_COUNTERS | PMCR_CYCLE_COUNTER_RESET;
	if (cycles_64_bit) flags |= PMCR_CYCLE_COUNTER_64_BIT;
	else flags = flags & ~PMCR_CYCLE_COUNTER_64_BIT;
	if (count_every_64) flags |= PMCR_CYCLE_COUNT_64;
	else flags = flags & ~PMCR_CYCLE_COUNT_64;
	pmcr_set(flags);
}

static inline void cntens_enable_pmccntr(void) {
	unsigned long flags = cntens_get();
	flags = flags | CNTENS_CYCLE_CTR;
	cntens_set(CNTENS_CYCLE_CTR);
}

static inline void pmccntr_enable(char cycles_64_bit, char count_every_64) {
	pmcr_enable_pmccntr(cycles_64_bit, count_every_64);
	cntens_enable_pmccntr();
}

static inline void pmccntr_enable_user(char cycles_64_bit, char count_every_64) {
	pmccntr_enable(cycles_64_bit, count_every_64);
	asm volatile ("MCR p15, 0, %0, c9, c14, 0" :: "r" (1));
}

static inline void pmccntr_enable_once(char cycles_64_bit, char count_every_64) {
	unsigned long flags;
	flags = pmcr_get();
	if ( !(flags & PMCR_ENABLE_COUNTERS) ) pmcr_enable_pmccntr(cycles_64_bit, count_every_64);;
	flags = cntens_get();
	if ( !(flags & CNTENS_CYCLE_CTR) ) cntens_enable_pmccntr();
}

static inline void pmccntr_enable_once_user(char cycles_64_bit, char count_every_64) {
	pmccntr_enable_once(cycles_64_bit, count_every_64);
	asm volatile ("MCR p15, 0, %0, c9, c14, 0" :: "r" (1));
}

static inline void pmccntr_reset(void) {
	unsigned long flags = pmcr_get();
	flags = flags | PMCR_CYCLE_COUNTER_RESET;
	pmcr_set(flags);
}

#endif //__ASMARM_ARCH_PERFMON_H