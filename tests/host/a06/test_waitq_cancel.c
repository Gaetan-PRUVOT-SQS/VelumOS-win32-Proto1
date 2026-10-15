#include <string.h>
#include "fake_sched.h"
#include "harness.h"
#include "velum/err.h"

static t_fuwaiter	g_w;

static void	*start_waiter(t_waitq *wq, t_spinlock *held, int *rec)
{
	void	*h;

	memset(&g_w, 0, sizeof(g_w));
	g_w.wq = wq;
	g_w.held = held;
	g_w.timeout = TIMEOUT_NONE;
	g_w.prio = PRIO_NORMAL;
	g_w.order = &rec[0];
	g_w.norder = &rec[1];
	rec[0] = -1;
	rec[1] = 0;
	h = fh_spawn(fu_waiter, &g_w);
	fu_wait_len(wq, 1);
	return (h);
}

static void	annulation_pendant(void)
{
	t_waitq	wq;
	void	*h;
	int		n[2];

	waitq_init(&wq);
	h = start_waiter(&wq, NULL, n);
	thread_cancel(&g_w.kt->t);
	fh_join(h);
	h_eq_i64("annulé pendant : E_CANCELED", g_w.rc, E_CANCELED);
	h_eq_u64("retiré de la file", fu_wq_len(&wq), 0);
}

static void	reveil_parasite(void)
{
	t_waitq	wq;
	void	*h;
	int		n[2];

	waitq_init(&wq);
	h = start_waiter(&wq, NULL, n);
	sched_unpark(g_w.kt, true);
	fh_sleep_us(5000);
	h_eq_i64("réveil parasite : toujours en attente", n[1], 0);
	h_eq_u64("toujours dans la file", fu_wq_len(&wq), 1);
	waitq_wake_one(&wq);
	fh_join(h);
	h_eq_i64("vrai réveil : rc 0", g_w.rc, 0);
	h_eq_i64("sorti une fois", n[1], 1);
}

static void	verrou_relache_pendant(void)
{
	t_waitq		wq;
	t_spinlock	held;
	void		*h;
	int			n[2];

	waitq_init(&wq);
	spin_init(&held, "objet");
	h = start_waiter(&wq, &held, n);
	h_true(spin_trylock(&held), "verrou relâché pendant l'attente");
	spin_unlock(&held);
	waitq_wake_one(&wq);
	fh_join(h);
	h_eq_i64("rc 0", g_w.rc, 0);
	h_true(g_w.held_ok, "verrou repris au retour");
	h_eq_u64("verrou libre à la fin", held.ticket, held.serving);
}

int	main(void)
{
	h_begin("a06/waitq_cancel");
	h_run("annulation pendant l'attente", annulation_pendant);
	h_run("réveil parasite ignoré", reveil_parasite);
	h_run("held relâché puis repris", verrou_relache_pendant);
	return (h_end());
}
