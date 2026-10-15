#include "errno.h"
#include "vtest.h"

static int	thr_run(t_vthreadfn fn, int n, uintptr_t *sum)
{
	t_vthread	*t[VTEST_THREADS];
	void		*res;
	int			i;
	int			ok;

	ok = 1;
	i = -1;
	while (++i < n)
		ok = ok && v_thread_create(&t[i], fn, (void *)(uintptr_t)(i + 1)) == 0;
	*sum = 0;
	while (i--)
	{
		ok = ok && v_thread_join(t[i], &res) == 0;
		*sum += (uintptr_t)res;
	}
	return (ok);
}

static void	thr_mutex(void)
{
	uintptr_t	sum;

	v_mutex_init(&g_shared.lock);
	g_shared.counter = 0;
	vtest_check(thr_run(vtest_worker_add, VTEST_THREADS, &sum),
		"fils: creation et join de 4 fils");
	vtest_check(sum == 1 + 2 + 3 + 4, "fils: valeurs de retour");
	vtest_check(g_shared.counter == (uint64_t)VTEST_THREADS * VTEST_LOOPS,
		"fils: mutex, compteur exact");
	vtest_check(v_mutex_unlock(&g_shared.lock) == E_PERM,
		"fils: unlock sans propriete refuse");
	v_mutex_destroy(&g_shared.lock);
}

static void	thr_event(void)
{
	t_vthread	*t[3];
	int			i;

	v_mutex_init(&g_shared.lock);
	g_shared.counter = 0;
	vtest_check(v_event_init(&g_shared.gate, 1, 0) == 0, "fils: evenement");
	i = -1;
	while (++i < 3)
		v_thread_create(&t[i], vtest_worker_gate, (void *)(uintptr_t)(i + 1));
	v_sleep(5 * V_NS_PER_MS);
	vtest_check(g_shared.counter == 0, "fils: les 3 fils attendent le signal");
	v_event_set(&g_shared.gate);
	while (i--)
		v_thread_join(t[i], NULL);
	vtest_check(g_shared.counter == 3, "fils: tous reveilles");
	v_event_destroy(&g_shared.gate);
	v_mutex_destroy(&g_shared.lock);
}

static void	thr_errno(void)
{
	uintptr_t	sum;

	errno = 77;
	vtest_check(thr_run(vtest_worker_errno, VTEST_THREADS, &sum)
		&& sum == VTEST_THREADS, "fils: errno propre a chaque fil");
	vtest_check(errno == 77, "fils: errno du fil principal intact");
}

void	vtest_thr(void)
{
	vtest_check(v_thread_self() == NULL,
		"fils: le fil principal n'a pas de t_vthread");
	thr_mutex();
	thr_event();
	thr_errno();
}
