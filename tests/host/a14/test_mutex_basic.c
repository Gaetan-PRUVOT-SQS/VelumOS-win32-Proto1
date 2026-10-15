#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/vsync.h"
#include "velum/vtls.h"

static void	mutex_zero_valid(void)
{
	static t_vmutex	m;

	fake_reset();
	fake_kernel_on();
	h_eq_i64("lock", v_mutex_lock(&m), 0);
	h_eq_u64("etat pris", m.state, V_MUTEX_LOCKED);
	h_eq_i64("unlock", v_mutex_unlock(&m), 0);
	h_eq_u64("etat libre", m.state, V_MUTEX_UNLOCKED);
	h_eq_u64("aucun evenement noyau sans contention", g_fsys.events_live, 0);
	v_mutex_destroy(&m);
}

static void	mutex_errors(void)
{
	t_vmutex	m;

	fake_reset();
	fake_kernel_on();
	v_tcb()->tid = 5;
	v_mutex_init(&m);
	h_eq_i64("unlock d'un mutex libre", v_mutex_unlock(&m), E_PERM);
	h_eq_i64("lock", v_mutex_lock(&m), 0);
	h_eq_i64("trylock d'un mutex pris", v_mutex_trylock(&m), E_BUSY);
	h_eq_i64("verrouillage recursif", v_mutex_lock(&m), E_DEADLK);
	h_eq_i64("unlock", v_mutex_unlock(&m), 0);
	h_eq_i64("trylock d'un mutex libre", v_mutex_trylock(&m), 0);
	h_eq_i64("unlock", v_mutex_unlock(&m), 0);
	v_mutex_destroy(&m);
}

static void	mutex_other_thread_unlock(void)
{
	t_vmutex	m;
	t_fsjob		job;
	pthread_t	th;

	fake_reset();
	fake_kernel_on();
	v_tcb()->tid = 5;
	v_mutex_init(&m);
	v_mutex_lock(&m);
	job.mutex = &m;
	job.tid = 6;
	job.rc = 99;
	pthread_create(&th, NULL, fs_unlock_once, &job);
	pthread_join(th, NULL);
	h_eq_i64("unlock par un autre fil refuse", job.rc, E_PERM);
	h_eq_u64("toujours pris", m.state, V_MUTEX_LOCKED);
	h_eq_i64("unlock par le proprietaire", v_mutex_unlock(&m), 0);
	v_mutex_destroy(&m);
}

static void	mutex_lazy_event(void)
{
	t_vmutex	m;
	t_fsjob		job;
	pthread_t	th;

	fake_reset();
	fake_kernel_on();
	v_tcb()->tid = 5;
	v_mutex_init(&m);
	v_mutex_lock(&m);
	job.mutex = &m;
	job.tid = 6;
	pthread_create(&th, NULL, fs_lock_once, &job);
	while (!m.event || fake_waiters(m.event) < 1)
		sched_yield();
	h_eq_u64("un evenement cree a la contention", g_fsys.events_live, 1);
	v_mutex_unlock(&m);
	pthread_join(th, NULL);
	h_eq_i64("le second fil a verrouille", job.rc, 0);
	v_mutex_destroy(&m);
	h_eq_u64("destroy ferme l'evenement", g_fsys.events_live, 0);
}

int	main(void)
{
	h_begin("a14/mutex_basic");
	h_run("mutex/partition : zero est un mutex libre", mutex_zero_valid);
	h_run("mutex/table de decision : erreurs", mutex_errors);
	h_run("mutex/table de decision : unlock par un autre fil",
		mutex_other_thread_unlock);
	h_run("mutex/etats : evenement paresseux", mutex_lazy_event);
	return (h_end());
}
