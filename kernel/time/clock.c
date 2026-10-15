#include "time_int.h"

static t_clock	g_clock;

t_clock	*clock_state(void)
{
	return (&g_clock);
}

bool	clock_ready(void)
{
	return (__atomic_load_n(&g_clock.ready, __ATOMIC_ACQUIRE));
}

void	clock_install(uint32_t source, uint64_t src_hz, uint64_t tsc_hz)
{
	g_clock.source = source;
	g_clock.src_hz = src_hz;
	g_clock.tsc_hz = tsc_hz;
	clockconv_init(&g_clock.to_ns, NS_PER_S, src_hz);
	if (tsc_hz)
		clockconv_init(&g_clock.tsc_per_ns, tsc_hz, NS_PER_S);
	if (source == CLOCK_SRC_TSC)
		g_clock.origin = tsc_read();
	else
		g_clock.origin = hpet_read();
	__atomic_store_n(&g_clock.ready, true, __ATOMIC_RELEASE);
}

uint64_t	time_now_ns(void)
{
	uint64_t	raw;

	if (!clock_ready())
		return (0);
	if (g_clock.source == CLOCK_SRC_TSC)
		raw = tsc_read();
	else
		raw = hpet_read();
	return (clockconv_apply(&g_clock.to_ns, raw - g_clock.origin));
}

uint64_t	tsc_hz(void)
{
	return (g_clock.tsc_hz);
}
