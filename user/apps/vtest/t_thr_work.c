#include "errno.h"
#include "vtest.h"

t_vtest_shared	g_shared;

void	*vtest_worker_add(void *arg)
{
	int	i;

	i = 0;
	while (i < VTEST_LOOPS)
	{
		v_mutex_lock(&g_shared.lock);
		g_shared.counter++;
		v_mutex_unlock(&g_shared.lock);
		i++;
	}
	return (arg);
}

void	*vtest_worker_gate(void *arg)
{
	v_event_wait(&g_shared.gate, TIMEOUT_INF);
	v_mutex_lock(&g_shared.lock);
	g_shared.counter++;
	v_mutex_unlock(&g_shared.lock);
	return (arg);
}

void	*vtest_worker_errno(void *arg)
{
	int	mine;

	mine = (int)(uintptr_t)arg;
	errno = mine;
	v_yield();
	v_sleep(V_NS_PER_MS);
	return ((void *)(uintptr_t)(errno == mine));
}
