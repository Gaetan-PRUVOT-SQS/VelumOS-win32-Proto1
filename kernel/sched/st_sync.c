#include "sched_st.h"
#include "velum/err.h"

static t_stpp	g_pp;

static void	pp_pinger(void *arg)
{
	t_stpp	*p;
	int		i;

	p = arg;
	i = 0;
	while (i < ST_PP_ROUNDS)
	{
		event_set(&p->ping);
		if (event_wait(&p->pong, ST_JOIN_NS) != 0)
			__atomic_add_fetch(&p->errors, 1, __ATOMIC_RELAXED);
		i++;
	}
}

static void	pp_ponger(void *arg)
{
	t_stpp	*p;
	int		i;

	p = arg;
	i = 0;
	while (i < ST_PP_ROUNDS)
	{
		if (event_wait(&p->ping, ST_JOIN_NS) != 0)
			__atomic_add_fetch(&p->errors, 1, __ATOMIC_RELAXED);
		else
			__atomic_add_fetch(&p->count, 1, __ATOMIC_RELAXED);
		event_set(&p->pong);
		i++;
	}
}

int	st_pingpong(void)
{
	t_thread	*a;
	t_thread	*b;
	int			rc;

	event_init(&g_pp.ping, false, false);
	event_init(&g_pp.pong, false, false);
	g_pp.count = 0;
	g_pp.errors = 0;
	if (st_spawn(pp_ponger, &g_pp, PRIO_NORMAL, &b) != 0)
		return (E_NOMEM);
	if (st_spawn(pp_pinger, &g_pp, PRIO_NORMAL, &a) != 0)
	{
		event_set(&g_pp.ping);
		return (E_NOMEM);
	}
	rc = st_join_unref(a);
	if (st_join_unref(b) != 0)
		rc = E_TIMEOUT;
	if (rc == 0 && (g_pp.errors != 0 || g_pp.count != ST_PP_ROUNDS))
		rc = E_RANGE;
	return (rc);
}
