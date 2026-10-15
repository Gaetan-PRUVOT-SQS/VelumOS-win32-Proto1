#include <string.h>
#include "init.h"

int	restart_allowed(t_restarts *r, uint64_t now_ns)
{
	uint32_t	i;
	uint32_t	recent;

	if (r->n > RESTART_MAX || r->next >= RESTART_MAX)
		return (0);
	recent = 0;
	i = 0;
	while (i < r->n)
	{
		if (now_ns >= r->stamp[i] && now_ns - r->stamp[i] < RESTART_WINDOW_NS)
			recent++;
		i++;
	}
	if (recent >= RESTART_MAX)
		return (0);
	r->stamp[r->next] = now_ns;
	r->next = (r->next + 1) % RESTART_MAX;
	if (r->n < RESTART_MAX)
		r->n++;
	return (1);
}

int	procs_have(const t_procinfo *ps, uint32_t n, const char *name)
{
	uint32_t	i;

	if (!ps || !name)
		return (0);
	i = 0;
	while (i < n && i < INIT_PROCS_MAX)
	{
		if (!strncmp(ps[i].name, name, sizeof(ps[i].name)))
			return (1);
		i++;
	}
	return (0);
}

uint64_t	killtest_timeout(const t_killtest *kt)
{
	if (kt->state == KT_ARMED || kt->state == KT_GRACE)
		return (KILLTEST_POLL_NS);
	return (TIMEOUT_INF);
}

int	killtest_requested(int argc, char **argv)
{
	int	i;

	if (!argv)
		return (0);
	i = 1;
	while (i < argc && argv[i])
	{
		if (!strcmp(argv[i], KILLTEST_ARG))
			return (1);
		i++;
	}
	return (0);
}

int	killtest_step(t_killtest *kt, int session_ready, uint64_t now_ns)
{
	if (kt->state != KT_ARMED && kt->state != KT_GRACE)
		return (0);
	if (kt->ticks >= KILLTEST_TICKS_MAX)
	{
		kt->state = KT_EXPIRED;
		return (0);
	}
	kt->ticks++;
	if (!session_ready)
	{
		kt->state = KT_ARMED;
		return (0);
	}
	if (kt->state == KT_ARMED || now_ns < kt->seen_ns)
	{
		kt->state = KT_GRACE;
		kt->seen_ns = now_ns;
		return (0);
	}
	if (now_ns - kt->seen_ns < KILLTEST_GRACE_NS)
		return (0);
	kt->state = KT_DONE;
	return (1);
}
