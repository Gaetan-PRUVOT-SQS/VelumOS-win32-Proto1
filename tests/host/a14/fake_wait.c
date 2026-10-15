#include "fake_int.h"
#include "fake_sys.h"
#include "velum/abi/abi_syscall.h"
#include "velum/err.h"

static int	fake_wait_for(t_fev *e, const uint64_t *a,
		const struct timespec *ts)
{
	uint64_t	gen;
	int			rc;

	gen = e->gen;
	rc = 0;
	e->waiters++;
	while (!e->signaled && e->gen == gen && !rc)
	{
		if (a[1] == TIMEOUT_INF)
			pthread_cond_wait(&g_fev.cond, &g_fev.lock);
		else if (pthread_cond_timedwait(&g_fev.cond, &g_fev.lock, ts))
			rc = E_TIMEOUT;
	}
	e->waiters--;
	if (!rc && e->signaled && !e->manual)
		e->signaled = 0;
	return (rc);
}

int64_t	fake_wait(const uint64_t *a)
{
	t_fev			*e;
	int				rc;
	struct timespec	ts;

	fake_deadline(a[1], &ts);
	pthread_mutex_lock(&g_fev.lock);
	e = fake_ev_get(a[0]);
	rc = E_BADF;
	if (e)
		rc = fake_wait_for(e, a, &ts);
	pthread_mutex_unlock(&g_fev.lock);
	return (rc);
}
