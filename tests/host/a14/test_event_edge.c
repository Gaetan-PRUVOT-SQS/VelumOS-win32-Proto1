#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/vsync.h"

static uint64_t	pulse_two_waiters(t_vevent *e)
{
	t_fsjob		job[2];
	pthread_t	th[2];
	uint64_t	count;
	int			i;

	count = 0;
	i = -1;
	while (++i < 2)
	{
		job[i].event = e;
		job[i].counter = &count;
		pthread_create(&th[i], NULL, fs_event_waiter, &job[i]);
	}
	fs_wait_waiters(e->handle, 2);
	v_event_pulse(e);
	while (i--)
		pthread_join(th[i], NULL);
	return (count);
}

static void	event_pulse(void)
{
	t_vevent	e;

	fake_reset();
	fake_kernel_on();
	v_event_init(&e, 1, 0);
	v_event_pulse(&e);
	h_eq_i64("pulse sans attendeur : perdu", v_event_wait(&e, 0), E_TIMEOUT);
	h_eq_u64("pulse : les deux attendeurs passent", pulse_two_waiters(&e), 2);
	h_eq_i64("pulse ne laisse pas signale", v_event_wait(&e, 0), E_TIMEOUT);
	v_event_destroy(&e);
}

static void	event_invalid(void)
{
	t_vevent	zero;

	zero.handle = 0;
	zero.manual = 0;
	h_eq_i64("init NULL", v_event_init(NULL, 0, 0), E_FAULT);
	h_eq_i64("set NULL", v_event_set(NULL), E_INVAL);
	h_eq_i64("reset NULL", v_event_reset(NULL), E_INVAL);
	h_eq_i64("pulse NULL", v_event_pulse(NULL), E_INVAL);
	h_eq_i64("wait NULL", v_event_wait(NULL, 0), E_INVAL);
	h_eq_i64("set handle nul", v_event_set(&zero), E_INVAL);
	h_eq_i64("wait handle nul", v_event_wait(&zero, 0), E_INVAL);
	v_event_destroy(NULL);
	v_event_destroy(&zero);
	fake_reset();
	g_fsys.ret = E_NFILE;
	h_eq_i64("init : le noyau refuse", v_event_init(&zero, 1, 0), E_NFILE);
}

static void	event_timeout(void)
{
	t_vevent		e;
	struct timespec	t0;
	struct timespec	t1;
	int64_t			elapsed;

	fake_reset();
	fake_kernel_on();
	v_event_init(&e, 0, 0);
	clock_gettime(CLOCK_REALTIME, &t0);
	h_eq_i64("wait 5 ms expire", v_event_wait(&e, 5000000), E_TIMEOUT);
	clock_gettime(CLOCK_REALTIME, &t1);
	elapsed = (int64_t)(t1.tv_sec - t0.tv_sec) * 1000000000
		+ (t1.tv_nsec - t0.tv_nsec);
	h_true(elapsed >= 4000000, "le delai a bien ete attendu");
	v_event_destroy(&e);
	v_event_destroy(&e);
	h_eq_u64("destroy idempotent", g_fsys.events_live, 0);
}

int	main(void)
{
	h_begin("a14/event_edge");
	h_run("event/limite : pulse", event_pulse);
	h_run("event/partition : arguments invalides", event_invalid);
	h_run("event/limite : delai", event_timeout);
	return (h_end());
}
