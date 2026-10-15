#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "harness.h"
#include "velum/err.h"
#include "velum/vsync.h"

static void	event_states(void)
{
	t_vevent	e;

	fake_reset();
	fake_kernel_on();
	v_event_init(&e, 1, 1);
	h_eq_i64("manuel signale : 1er wait", v_event_wait(&e, 0), 0);
	h_eq_i64("manuel signale : reste signale", v_event_wait(&e, 0), 0);
	v_event_reset(&e);
	h_eq_i64("manuel apres reset", v_event_wait(&e, 0), E_TIMEOUT);
	v_event_destroy(&e);
	v_event_init(&e, 0, 1);
	h_eq_i64("auto signale : 1er wait", v_event_wait(&e, 0), 0);
	h_eq_i64("auto : rearme apres un reveil", v_event_wait(&e, 0), E_TIMEOUT);
	v_event_set(&e);
	h_eq_i64("auto : set puis wait", v_event_wait(&e, 0), 0);
	v_event_destroy(&e);
	h_eq_u64("evenements fermes", g_fsys.events_live, 0);
}

static void	event_waiters(t_vevent *e, int n, int *passed)
{
	pthread_t	th[3];
	t_fsjob		job[3];
	uint64_t	count;
	int			i;

	count = 0;
	i = -1;
	while (++i < n)
	{
		job[i].event = e;
		job[i].counter = &count;
		pthread_create(&th[i], NULL, fs_event_waiter, &job[i]);
	}
	h_true(fs_wait_waiters(e->handle, n), "tous les fils attendent");
	v_event_set(e);
	usleep(30000);
	*passed = (int)__atomic_load_n(&count, __ATOMIC_SEQ_CST);
	if (!e->manual)
		v_event_set(e);
	if (!e->manual && n == 3)
		v_event_set(e);
	while (i--)
		pthread_join(th[i], NULL);
}

static void	event_auto_single_wake(void)
{
	t_vevent	e;
	int			passed;

	fake_reset();
	fake_kernel_on();
	v_event_init(&e, 0, 0);
	event_waiters(&e, 2, &passed);
	h_eq_i64("auto : un seul reveil par set", passed, 1);
	v_event_destroy(&e);
}

static void	event_manual_wakes_all(void)
{
	t_vevent	e;
	int			passed;

	fake_reset();
	fake_kernel_on();
	v_event_init(&e, 1, 0);
	event_waiters(&e, 3, &passed);
	h_eq_i64("manuel : tous reveilles", passed, 3);
	h_eq_i64("manuel : toujours signale", v_event_wait(&e, 0), 0);
	v_event_destroy(&e);
}

int	main(void)
{
	h_begin("a14/event");
	h_run("event/transitions : etats initiaux", event_states);
	h_run("event/table de decision : automatique", event_auto_single_wake);
	h_run("event/table de decision : manuel", event_manual_wakes_all);
	return (h_end());
}
