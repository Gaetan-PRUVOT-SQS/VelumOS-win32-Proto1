#include "apic_int.h"
#include "velum/cpu.h"
#include "velum/msr.h"
#include "velum/timer.h"

#define CALIBRATE_NS 10000000ull
#define NS_PER_S 1000000000ull

void	lapic_timer_deadline_mode(void)
{
	lapic_write(LAPIC_LVT_TIMER, LAPIC_TIMER_TSCDL | VEC_TIMER);
	__asm__ volatile ("mfence" : : : "memory");
}

void	lapic_timer_oneshot_mode(void)
{
	lapic_write(LAPIC_TIMER_DIV, LAPIC_DIV_16);
	lapic_write(LAPIC_LVT_TIMER, VEC_TIMER);
}

void	lapic_timer_set_count(uint32_t count)
{
	lapic_write(LAPIC_TIMER_INIT, count);
}

void	lapic_timer_set_deadline(uint64_t tsc)
{
	msr_write(MSR_TSC_DEADLINE, tsc);
}

uint64_t	lapic_timer_calibrate(void)
{
	uint64_t	t0;
	uint64_t	t1;
	uint32_t	cur;

	lapic_write(LAPIC_TIMER_DIV, LAPIC_DIV_16);
	lapic_write(LAPIC_LVT_TIMER, LAPIC_LVT_MASKED | VEC_TIMER);
	t0 = time_now_ns();
	lapic_write(LAPIC_TIMER_INIT, 0xffffffffu);
	time_delay_ns(CALIBRATE_NS);
	cur = lapic_read(LAPIC_TIMER_CUR);
	t1 = time_now_ns();
	lapic_write(LAPIC_TIMER_INIT, 0);
	if (t1 <= t0 || cur == 0)
		return (0);
	return ((uint64_t)(0xffffffffu - cur) * NS_PER_S / (t1 - t0));
}
