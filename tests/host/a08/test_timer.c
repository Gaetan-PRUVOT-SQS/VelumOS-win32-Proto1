#include "harness.h"
#include "fake.h"

static uint64_t	g_deadline;

static int	armed(void)
{
	int	i;
	int	n;

	n = 0;
	i = 0;
	while (i < FK_TIMERS)
	{
		if (g_fk.timers[i].armed)
			g_deadline = g_fk.timers[i].deadline;
		n += g_fk.timers[i++].armed;
	}
	return (n);
}

static void	oneshot_auto(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_TIMER_CREATE, 0, 0, 0);
	h_true(h > 0, "creation");
	h_eq_i64("armement", fk_call(SYS_TIMER_SET, h, g_fk.now + 1000, 0), 0);
	h_eq_i64("pas encore", fk_call(SYS_WAIT, h, 0, 0), E_TIMEOUT);
	g_fk.now += 1000;
	fk_fire_due();
	h_eq_i64("echue", fk_call(SYS_WAIT, h, 0, 0), 0);
	h_eq_i64("auto consommee", fk_call(SYS_WAIT, h, 0, 0), E_TIMEOUT);
	h_eq_i64("pas de rearmement", armed(), 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	periodic(void)
{
	t_process	*p;
	int64_t		h;

	fk_reset();
	p = fk_proc_new(0);
	g_fk.cur = p;
	h = fk_call(SYS_TIMER_CREATE, 1, 0, 0);
	h_eq_i64("periode < 1 ms", fk_call(SYS_TIMER_SET, h, 5000, 999999),
		E_INVAL);
	h_eq_i64("periode 1 ms", fk_call(SYS_TIMER_SET, h, 1001000, 1000000), 0);
	g_fk.now = 1001000;
	fk_fire_due();
	h_true(armed() == 1 && g_deadline == 2001000, "periode suivante");
	g_fk.now = 9000000;
	fk_fire_due();
	h_true(armed() == 1 && g_deadline == 10000000, "retard : sautees");
	h_eq_i64("manuelle reste signalee", fk_call(SYS_WAIT, h, 0, 0), 0);
	fk_call(SYS_TIMER_SET, h, 0, 0);
	h_eq_i64("annulee", fk_call(SYS_WAIT, h, 0, 0), E_TIMEOUT);
	h_eq_i64("plus rien d'arme", armed(), 0);
	fk_proc_free(p);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

static void	stale_and_destroy(void)
{
	t_object	*t;
	void		*old;

	fk_reset();
	tmr_create(0, &t);
	tmr_set(t, 5000, 0);
	old = g_fk.timers[0].ctx;
	tmr_set(t, 9000, 0);
	h_eq_i64("ancien rappel annule", armed(), 1);
	tmr_fire(old);
	h_true(!sig_signaled(t), "rappel perime sans effet");
	h_eq_i64("type", tmr_set(NULL, 1, 0), E_INVAL);
	obj_unref(t);
	h_eq_i64("destruction annule", armed(), 0);
	tmr_fire(g_fk.timers[0].ctx);
	h_eq_i64("rappel apres destruction sans effet", fk_heap_total(), 0);
	g_fk.heap_fail = 2;
	h_eq_i64("objet sans memoire", tmr_create(0, &t), E_NOMEM);
	h_eq_i64("tas rendu", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/timer");
	h_run("oneshot_auto", oneshot_auto);
	h_run("periodic", periodic);
	h_run("stale_and_destroy", stale_and_destroy);
	return (h_end());
}
