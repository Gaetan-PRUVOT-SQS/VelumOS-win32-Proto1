#include "fake.h"
#include "velum/timer.h"

t_ftime	g_ftime;

void	ftime_reset(void)
{
	g_ftime.now = 1000000000ull;
	g_ftime.step = 1000;
}

uint64_t	time_now_ns(void)
{
	g_ftime.now += g_ftime.step;
	return (g_ftime.now);
}
