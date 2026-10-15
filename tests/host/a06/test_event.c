#include <string.h>
#include "fake_sched.h"
#include "harness.h"
#include "velum/err.h"

static t_fuwaiter	g_w[3];
static t_event		g_ev;

static void	evenement_auto(void)
{
	event_init(&g_ev, false, false);
	h_eq_i64("non signalé : E_TIMEOUT", event_wait(&g_ev, 0), E_TIMEOUT);
	event_set(&g_ev);
	h_true(g_ev.signaled, "signalé sans attendeur");
	h_eq_i64("consommé", event_wait(&g_ev, 0), 0);
	h_eq_i64("remis à zéro tout seul", event_wait(&g_ev, 0), E_TIMEOUT);
	event_init(&g_ev, false, true);
	h_eq_i64("état initial signalé", event_wait(&g_ev, 0), 0);
}

static void	evenement_manuel(void)
{
	event_init(&g_ev, true, false);
	event_set(&g_ev);
	h_eq_i64("manuel 1", event_wait(&g_ev, 0), 0);
	h_eq_i64("manuel 2", event_wait(&g_ev, 0), 0);
	event_reset(&g_ev);
	h_eq_i64("après reset : E_TIMEOUT", event_wait(&g_ev, 0), E_TIMEOUT);
}

static void	spawn_ev(int count, uint64_t timeout, void **h, int *rec)
{
	int	i;

	i = 0;
	while (i < count)
	{
		memset(&g_w[i], 0, sizeof(g_w[i]));
		g_w[i].ev = &g_ev;
		g_w[i].timeout = timeout;
		g_w[i].tag = i;
		g_w[i].order = rec;
		g_w[i].norder = &rec[3];
		h[i] = fh_spawn(fu_ev_waiter, &g_w[i]);
		fu_wait_len(&g_ev.wq, (uint32_t)i + 1);
		i++;
	}
}

static void	reveils_auto_et_manuel(void)
{
	void	*h[3];
	int		rec[4];

	event_init(&g_ev, false, false);
	rec[3] = 0;
	spawn_ev(2, 200000000ull, h, rec);
	event_set(&g_ev);
	fh_join(h[0]);
	fh_join(h[1]);
	h_true(g_w[0].rc == 0 && g_w[1].rc == E_TIMEOUT,
		"auto : un seul réveillé, le premier arrivé");
	h_true(!g_ev.signaled, "auto : consommé par le réveillé");
	event_init(&g_ev, true, false);
	rec[3] = 0;
	spawn_ev(3, TIMEOUT_NONE, h, rec);
	event_set(&g_ev);
	fh_join(h[0]);
	fh_join(h[1]);
	fh_join(h[2]);
	h_true(rec[3] == 3 && g_w[0].rc == 0 && g_w[1].rc == 0 && g_w[2].rc == 0,
		"manuel : tous réveillés");
}

int	main(void)
{
	h_begin("a06/event");
	h_run("événement automatique", evenement_auto);
	h_run("événement manuel", evenement_manuel);
	h_run("réveils auto et manuel", reveils_auto_et_manuel);
	return (h_end());
}
