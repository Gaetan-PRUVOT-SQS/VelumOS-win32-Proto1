#include "sched_st.h"
#include "velum/err.h"
#include "velum/klog.h"

static t_stmx	g_mx;

static void	mx_worker(void *arg)
{
	t_stmx	*x;
	int32_t	v;
	int		i;

	x = arg;
	i = 0;
	while (i < ST_MUTEX_LOOPS)
	{
		mutex_lock(&x->m);
		v = x->counter;
		if ((i & 63) == 0)
			sched_yield();
		x->counter = v + 1;
		mutex_unlock(&x->m);
		i++;
	}
}

static int	mx_join_all(t_thread **t, int n)
{
	int	rc;
	int	i;

	rc = 0;
	i = 0;
	while (i < n)
	{
		if (st_join_unref(t[i]) != 0)
			rc = E_TIMEOUT;
		i++;
	}
	return (rc);
}

int	st_mutex(void)
{
	t_thread	*t[ST_MUTEX_THREADS];
	int			n;
	int			rc;

	mutex_init(&g_mx.m, "autotest");
	g_mx.counter = 0;
	n = 0;
	while (n < ST_MUTEX_THREADS)
	{
		if (st_spawn(mx_worker, &g_mx, PRIO_NORMAL, &t[n]) != 0)
			break ;
		n++;
	}
	rc = mx_join_all(t, n);
	if (rc == 0 && n < ST_MUTEX_THREADS)
		rc = E_NOMEM;
	klog_info("sched: mutex, compteur %d sur %d", g_mx.counter,
		ST_MUTEX_THREADS * ST_MUTEX_LOOPS);
	if (rc == 0 && g_mx.counter != ST_MUTEX_THREADS * ST_MUTEX_LOOPS)
		rc = E_RANGE;
	return (rc);
}
