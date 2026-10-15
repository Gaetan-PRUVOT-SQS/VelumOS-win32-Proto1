#include "fake_int.h"
#include "fake_sys.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"

t_fevs	g_fev = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, {{0}}};

t_fev	*fake_ev_get(uint64_t h)
{
	if (h < FAKE_EV_BASE || h >= FAKE_EV_BASE + FAKE_EVENTS)
		return (NULL);
	if (!g_fev.ev[h - FAKE_EV_BASE].used)
		return (NULL);
	return (&g_fev.ev[h - FAKE_EV_BASE]);
}

int64_t	fake_event_create(const uint64_t *a)
{
	int	i;

	pthread_mutex_lock(&g_fev.lock);
	i = 0;
	while (i < FAKE_EVENTS && g_fev.ev[i].used)
		i++;
	if (i == FAKE_EVENTS)
	{
		pthread_mutex_unlock(&g_fev.lock);
		return (E_NFILE);
	}
	g_fev.ev[i].used = 1;
	g_fev.ev[i].manual = a[0] != 0;
	g_fev.ev[i].signaled = a[1] != 0;
	g_fev.ev[i].waiters = 0;
	g_fev.ev[i].gen = 0;
	g_fsys.events_live++;
	pthread_mutex_unlock(&g_fev.lock);
	return (FAKE_EV_BASE + i);
}

int64_t	fake_event_op(const uint64_t *a)
{
	t_fev	*e;

	pthread_mutex_lock(&g_fev.lock);
	e = fake_ev_get(a[0]);
	if (!e || a[1] > EV_PULSE)
	{
		pthread_mutex_unlock(&g_fev.lock);
		return (E_BADF);
	}
	if (a[1] == EV_SET)
		e->signaled = 1;
	else if (a[1] == EV_RESET)
		e->signaled = 0;
	else if (e->waiters && e->manual)
		e->gen++;
	else if (e->waiters)
		e->signaled = 1;
	pthread_cond_broadcast(&g_fev.cond);
	pthread_mutex_unlock(&g_fev.lock);
	return (0);
}

int64_t	fake_event_close(uint64_t h)
{
	t_fev	*e;

	pthread_mutex_lock(&g_fev.lock);
	e = fake_ev_get(h);
	if (e)
	{
		e->used = 0;
		g_fsys.events_live--;
	}
	pthread_mutex_unlock(&g_fev.lock);
	if (!e)
		return (E_BADF);
	return (0);
}

int	fake_waiters(uint64_t h)
{
	t_fev	*e;
	int		n;

	pthread_mutex_lock(&g_fev.lock);
	e = fake_ev_get(h);
	n = -1;
	if (e)
		n = e->waiters;
	pthread_mutex_unlock(&g_fev.lock);
	return (n);
}
