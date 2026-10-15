#include <string.h>
#include "fake_sched.h"
#include "harness.h"

static t_fuwaiter	g_w[2];
static t_mutex		g_m;

static void	*pi_start(int i, int32_t prio, int *order, int *n)
{
	void	*h;

	memset(&g_w[i], 0, sizeof(g_w[i]));
	g_w[i].m = &g_m;
	g_w[i].prio = prio;
	g_w[i].tag = i;
	g_w[i].order = order;
	g_w[i].norder = n;
	h = fh_spawn(fu_pi_waiter, &g_w[i]);
	fu_wait_len(&g_m.wq, (uint32_t)i + 1);
	return (h);
}

static void	heritage_simple(void)
{
	t_kthread	*me;
	void		*h;
	int			order[2];
	int			n;

	me = sched_kself();
	n = 0;
	mutex_init(&g_m, "pi");
	mutex_lock(&g_m);
	h = pi_start(0, PRIO_HIGH, order, &n);
	h_eq_i64("propriétaire monté à PRIO_HIGH", me->t.prio, PRIO_HIGH);
	mutex_unlock(&g_m);
	h_eq_i64("propriétaire redescendu", me->t.prio, PRIO_NORMAL);
	fh_join(h);
	h_true(g_w[0].owner_ok, "passage de main au fil réveillé");
	h_true(g_m.owner == NULL, "libre à la fin");
}

static void	heritage_passe_au_suivant(void)
{
	void	*h[2];
	int		order[2];
	int		n;

	n = 0;
	mutex_init(&g_m, "pi2");
	mutex_lock(&g_m);
	h[0] = pi_start(0, 10, order, &n);
	h[1] = pi_start(1, PRIO_HIGH, order, &n);
	h_eq_i64("propriétaire au plus haut attendeur", sched_kself()->t.prio,
		PRIO_HIGH);
	mutex_unlock(&g_m);
	fh_join(h[0]);
	fh_join(h[1]);
	h_true(n == 2 && order[0] == 0 && order[1] == 1, "ordre FIFO");
	h_eq_i64("le suivant hérite du restant", g_w[0].seen_prio, PRIO_HIGH);
	h_eq_i64("le dernier garde sa base", g_w[1].seen_prio, PRIO_HIGH);
	h_eq_i64("premier redescendu", g_w[0].kt->t.prio, 10);
}

static void	annulation_ignoree(void)
{
	void	*h;
	int		order[1];
	int		n;

	n = 0;
	mutex_init(&g_m, "pi3");
	mutex_lock(&g_m);
	h = pi_start(0, PRIO_NORMAL, order, &n);
	thread_cancel(&g_w[0].kt->t);
	fh_sleep_us(5000);
	h_eq_u64("annulé mais toujours en attente du mutex",
		fu_wq_len(&g_m.wq), 1);
	mutex_unlock(&g_m);
	fh_join(h);
	h_true(g_w[0].owner_ok && n == 1, "a eu le mutex malgré l'annulation");
}

int	main(void)
{
	h_begin("a06/mutex_pi");
	h_run("héritage simple", heritage_simple);
	h_run("héritage transmis au suivant", heritage_passe_au_suivant);
	h_run("mutex non annulable", annulation_ignoree);
	return (h_end());
}
