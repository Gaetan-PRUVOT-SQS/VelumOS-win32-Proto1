#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/vobj.h"
#include "velum/vsync.h"
#include "velum/vtls.h"

#define STRESS_THREADS 8
#define STRESS_LOOPS 200000

static void	stress_run(t_vmutex *m, uint64_t *counter)
{
	pthread_t	th[STRESS_THREADS];
	t_fsjob		job[STRESS_THREADS];
	int			i;

	i = -1;
	while (++i < STRESS_THREADS)
	{
		job[i].mutex = m;
		job[i].counter = counter;
		job[i].loops = STRESS_LOOPS;
		job[i].tid = (uint32_t)i + 10;
		pthread_create(&th[i], NULL, fs_mutex_worker, &job[i]);
	}
	while (i--)
		pthread_join(th[i], NULL);
}

static void	mutex_stress(void)
{
	t_vmutex	m;
	uint64_t	counter;

	fake_reset();
	fake_kernel_on();
	g_fsys.record = 0;
	alarm(240);
	v_mutex_init(&m);
	counter = 0;
	stress_run(&m, &counter);
	h_eq_u64("compteur exact", counter,
		(uint64_t)STRESS_THREADS * STRESS_LOOPS);
	h_eq_u64("mutex libre a la fin", m.state, V_MUTEX_UNLOCKED);
	v_mutex_destroy(&m);
	alarm(0);
}

static int	saturate_events(int64_t *h)
{
	int	n;

	n = 0;
	while (n < 256)
	{
		h[n] = v_event_create(0, 0);
		if (h[n] <= 0)
			break ;
		n++;
	}
	return (n);
}

static void	mutex_event_failure_fallback(void)
{
	t_vmutex	m;
	t_fsjob		job;
	pthread_t	th;
	int64_t		h[256];
	int			n;

	fake_reset();
	fake_kernel_on();
	v_tcb()->tid = 5;
	n = saturate_events(h);
	v_mutex_init(&m);
	v_mutex_lock(&m);
	job.mutex = &m;
	job.tid = 6;
	pthread_create(&th, NULL, fs_lock_once, &job);
	usleep(20000);
	h_true(g_fsys.yield_calls > 0, "repli sur v_yield sans evenement");
	v_mutex_unlock(&m);
	pthread_join(th, NULL);
	h_eq_i64("le second fil a fini par verrouiller", job.rc, 0);
	while (n--)
		v_close((t_handle)h[n]);
}

int	main(void)
{
	h_begin("a14/mutex_stress");
	h_run("mutex/concurrence : 8 fils x 200 000", mutex_stress);
	h_run("mutex/injection : evenement impossible",
		mutex_event_failure_fallback);
	return (h_end());
}
