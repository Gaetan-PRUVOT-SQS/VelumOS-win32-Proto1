#include "fake.h"

uint64_t	time_now_ns(void)
{
	return (g_fk.now);
}

int64_t	timer_arm(uint64_t deadline_ns, t_timerfn fn, void *ctx)
{
	uint32_t	i;

	i = 0;
	while (i < FK_TIMERS && g_fk.timers[i].armed)
		i++;
	if (i == FK_TIMERS)
		return (E_NOMEM);
	g_fk.timers[i].armed = 1;
	g_fk.timers[i].deadline = deadline_ns;
	g_fk.timers[i].fn = fn;
	g_fk.timers[i].ctx = ctx;
	g_fk.timers[i].id = g_fk.next_tid;
	g_fk.next_tid++;
	return (g_fk.timers[i].id);
}

bool	timer_cancel(int64_t id)
{
	uint32_t	i;

	i = 0;
	while (i < FK_TIMERS)
	{
		if (g_fk.timers[i].armed && g_fk.timers[i].id == id)
		{
			g_fk.timers[i].armed = 0;
			return (true);
		}
		i++;
	}
	return (false);
}

void	fk_fire_due(void)
{
	uint32_t	i;

	i = 0;
	while (i < FK_TIMERS)
	{
		if (g_fk.timers[i].armed && g_fk.timers[i].deadline <= g_fk.now)
		{
			g_fk.timers[i].armed = 0;
			g_fk.timers[i].fn(g_fk.timers[i].ctx);
		}
		i++;
	}
}
