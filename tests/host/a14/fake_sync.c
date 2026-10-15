#include <unistd.h>
#include "fake_sync.h"
#include "fake_sys.h"
#include "velum/vtls.h"

void	*fs_mutex_worker(void *arg)
{
	t_fsjob		*j;
	uint32_t	i;

	j = arg;
	v_tcb()->tid = j->tid;
	i = 0;
	while (i < j->loops)
	{
		v_mutex_lock(j->mutex);
		(*j->counter)++;
		v_mutex_unlock(j->mutex);
		i++;
	}
	return (NULL);
}

void	*fs_spin_worker(void *arg)
{
	t_fsjob		*j;
	uint32_t	i;

	j = arg;
	i = 0;
	while (i < j->loops)
	{
		v_spin_lock(j->spin);
		(*j->counter)++;
		v_spin_unlock(j->spin);
		i++;
	}
	return (NULL);
}

void	*fs_lock_once(void *arg)
{
	t_fsjob	*j;

	j = arg;
	v_tcb()->tid = j->tid;
	j->rc = v_mutex_lock(j->mutex);
	if (j->rc == 0)
		v_mutex_unlock(j->mutex);
	return (NULL);
}

void	*fs_unlock_once(void *arg)
{
	t_fsjob	*j;

	j = arg;
	v_tcb()->tid = j->tid;
	j->rc = v_mutex_unlock(j->mutex);
	return (NULL);
}

void	*fs_event_waiter(void *arg)
{
	t_fsjob	*j;

	j = arg;
	j->rc = v_event_wait(j->event, TIMEOUT_INF);
	__atomic_fetch_add(j->counter, 1, __ATOMIC_SEQ_CST);
	return (NULL);
}
