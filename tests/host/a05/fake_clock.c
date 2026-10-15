#include "fake.h"
#include "time_int.h"
#include "velum/irqflags.h"

t_fakeclock	g_fclock;

void	fclock_reset(bool ready, uint64_t step)
{
	g_fclock.now = 1000;
	g_fclock.step = step;
	g_fclock.relax = 0;
	g_fclock.delays = 0;
	g_fclock.ready = ready;
}

bool	clock_ready(void)
{
	return (g_fclock.ready);
}

uint64_t	time_now_ns(void)
{
	uint64_t	v;

	v = g_fclock.now;
	g_fclock.now += g_fclock.step;
	return (v);
}

void	cpu_relax(void)
{
	g_fclock.relax++;
}

void	time_delay_ns(uint64_t ns)
{
	g_fclock.delays++;
	g_fclock.now += ns;
}
