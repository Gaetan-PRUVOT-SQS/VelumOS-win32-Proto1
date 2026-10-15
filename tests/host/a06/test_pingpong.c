#include "fake_sched.h"
#include "harness.h"

#define PP_ROUNDS 10000

static t_futest	g_t;

static void	pp_ponger(void *arg)
{
	t_futest	*t;
	int			i;

	t = arg;
	i = 0;
	while (i < PP_ROUNDS)
	{
		if (event_wait(&t->ping, 2000000000ull) != 0)
			__atomic_add_fetch(&t->errors, 1, __ATOMIC_RELAXED);
		else
			t->counter++;
		event_set(&t->pong);
		i++;
	}
}

static void	pingpong_evenements(void)
{
	void	*h;
	int		i;

	event_init(&g_t.ping, false, false);
	event_init(&g_t.pong, false, false);
	g_t.counter = 0;
	g_t.errors = 0;
	h = fh_spawn(pp_ponger, &g_t);
	i = 0;
	while (i < PP_ROUNDS)
	{
		event_set(&g_t.ping);
		if (event_wait(&g_t.pong, 2000000000ull) != 0)
			__atomic_add_fetch(&g_t.errors, 1, __ATOMIC_RELAXED);
		i++;
	}
	fh_join(h);
	h_eq_u64("10 000 allers-retours", g_t.counter, PP_ROUNDS);
	h_eq_i64("aucun délai dépassé", g_t.errors, 0);
}

int	main(void)
{
	h_begin("a06/pingpong");
	h_run("ping-pong par événements", pingpong_evenements);
	return (h_end());
}
