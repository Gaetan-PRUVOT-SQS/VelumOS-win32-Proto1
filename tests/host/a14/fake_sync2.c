#include <unistd.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "velum/err.h"
#include "velum/vtls.h"

void	*fs_join_worker(void *arg)
{
	t_fsjob	*j;

	j = arg;
	j->rc = v_thread_join(*(t_vthread **)j->counter, NULL);
	return (NULL);
}

int	fs_wait_waiters(uint64_t handle, int count)
{
	int	tries;

	tries = 0;
	while (tries < 2000)
	{
		if (fake_waiters(handle) >= count)
			return (1);
		usleep(1000);
		tries++;
	}
	return (0);
}

void	*fs_gate_thread(void *arg)
{
	t_vevent	*gate;

	gate = arg;
	v_event_wait(gate, TIMEOUT_INF);
	return (arg);
}

int	fs_wait_threads(void)
{
	int	tries;

	tries = 0;
	while (tries < 3000)
	{
		if (__atomic_load_n(&g_fsys.threads_alive, __ATOMIC_SEQ_CST) == 0)
			return (1);
		usleep(1000);
		tries++;
	}
	return (0);
}

void	*fs_tid_get(void *arg)
{
	*(int64_t *)arg = v_tcb()->tid;
	return (NULL);
}
