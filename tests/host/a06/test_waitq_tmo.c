#include <string.h>
#include "fake_sched.h"
#include "harness.h"
#include "velum/err.h"

#define RACE_ROUNDS 300

static t_fuwaiter	g_w;

static void	delai_expire(void)
{
	t_waitq		wq;
	uint64_t	start;
	uint64_t	took;
	int			rc;

	waitq_init(&wq);
	start = fh_now_ns();
	rc = waitq_wait(&wq, NULL, 20000000ull);
	took = fh_now_ns() - start;
	h_eq_i64("délai : E_TIMEOUT", rc, E_TIMEOUT);
	h_true(took >= 20000000ull && took < 1000000000ull, "durée 20 ms");
	h_eq_u64("plus dans la file", fu_wq_len(&wq), 0);
	h_eq_u64("aucun verrou tenu", lockdep_depth(), 0);
}

static void	delai_zero_et_annule(void)
{
	t_waitq		wq;
	t_spinlock	held;

	waitq_init(&wq);
	spin_init(&held, "objet");
	spin_lock(&held);
	h_eq_i64("délai 0 : E_TIMEOUT", waitq_wait(&wq, &held, 0), E_TIMEOUT);
	h_eq_u64("verrou toujours tenu", sched_preempt_count(), 1);
	thread_cancel(&sched_kself()->t);
	h_true(thread_cancel_pending(), "annulation visible");
	h_eq_i64("annulé avant : E_CANCELED",
		waitq_wait(&wq, &held, TIMEOUT_NONE), E_CANCELED);
	h_eq_u64("verrou toujours tenu après refus", sched_preempt_count(), 1);
	spin_unlock(&held);
	thread_cancel(NULL);
	fake_reset_self();
	h_true(!thread_cancel_pending(), "annulation effacée");
}

static void	echec_minuterie(void)
{
	t_waitq	wq;

	waitq_init(&wq);
	fake_timer_fail(true);
	h_eq_i64("minuterie refusée : E_NOMEM",
		waitq_wait(&wq, NULL, 20000000ull), E_NOMEM);
	fake_timer_fail(false);
	h_eq_u64("plus dans la file", fu_wq_len(&wq), 0);
	h_eq_u64("aucun verrou tenu", lockdep_depth(), 0);
}

static void	course_reveil_delai(void)
{
	t_waitq	wq;
	void	*h;
	int		i;
	int		ok;

	ok = 0;
	i = 0;
	while (i < RACE_ROUNDS)
	{
		waitq_init(&wq);
		memset(&g_w, 0, sizeof(g_w));
		g_w.wq = &wq;
		g_w.timeout = 1000000ull;
		g_w.rc = 1;
		h = fh_spawn(fu_waiter, &g_w);
		fh_sleep_us((uint64_t)(i % 4) * 400);
		waitq_wake_one(&wq);
		fh_join(h);
		ok += ((g_w.rc == 0 || g_w.rc == E_TIMEOUT) && fu_wq_len(&wq) == 0);
		i++;
	}
	h_eq_i64("300 courses réveil/délai sans perte ni reste", ok, RACE_ROUNDS);
}

int	main(void)
{
	h_begin("a06/waitq_tmo");
	h_run("délai qui expire", delai_expire);
	h_run("délai 0 et annulation préalable", delai_zero_et_annule);
	h_run("minuterie qui ne s'arme pas", echec_minuterie);
	h_run("course réveil contre délai", course_reveil_delai);
	return (h_end());
}
