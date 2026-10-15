#include "velum/libk.h"
#include "../common/timefmt.h"
#include "limiter.h"

void	lim_init(t_limiters *l)
{
	memset(l, 0, sizeof(*l));
}

uint64_t	lim_delay_ns(uint32_t failures)
{
	uint64_t	seconds;

	if (failures == 0)
		return (0);
	if (failures > 6)
		failures = 6;
	seconds = (uint64_t)LIM_DELAY_FIRST_S << (failures - 1);
	if (seconds > LIM_DELAY_MAX_S)
		seconds = LIM_DELAY_MAX_S;
	return (seconds * NS_PER_SEC);
}

uint32_t	lim_remaining_s(const t_limiters *l, uint32_t idx, uint64_t now_ns)
{
	uint64_t	left;

	if (idx >= ACC_MAX || l->slot[idx].blocked_until_ns <= now_ns)
		return (0);
	left = l->slot[idx].blocked_until_ns - now_ns;
	return ((uint32_t)((left + NS_PER_SEC - 1) / NS_PER_SEC));
}

void	lim_fail(t_limiters *l, uint32_t idx, uint64_t now_ns)
{
	t_limiter	*s;
	uint64_t	delay;

	if (idx >= ACC_MAX)
		return ;
	s = &l->slot[idx];
	if (s->failures < LIM_FAILURES_MAX)
		s->failures++;
	delay = lim_delay_ns(s->failures);
	s->blocked_until_ns = UINT64_MAX;
	if (now_ns <= UINT64_MAX - delay)
		s->blocked_until_ns = now_ns + delay;
}

void	lim_reset(t_limiters *l, uint32_t idx)
{
	if (idx >= ACC_MAX)
		return ;
	l->slot[idx].failures = 0;
	l->slot[idx].blocked_until_ns = 0;
}
