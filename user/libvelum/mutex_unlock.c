#include "velum/err.h"
#include "velum/vobj.h"
#include "velum/vsync.h"
#include "velum/vtls.h"

int	v_mutex_unlock(t_vmutex *mutex)
{
	uint32_t	tid;
	uint32_t	h;

	tid = v_tcb()->tid;
	if (__atomic_load_n(&mutex->state, __ATOMIC_RELAXED) == V_MUTEX_UNLOCKED
		|| __atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) != tid)
		return (E_PERM);
	__atomic_store_n(&mutex->owner, 0, __ATOMIC_RELAXED);
	if (__atomic_exchange_n(&mutex->state, V_MUTEX_UNLOCKED,
			__ATOMIC_RELEASE) == V_MUTEX_CONTENDED)
	{
		h = __atomic_load_n(&mutex->event, __ATOMIC_ACQUIRE);
		if (h)
			v_event_op(h, EV_SET);
	}
	return (0);
}

void	v_mutex_destroy(t_vmutex *mutex)
{
	if (!mutex)
		return ;
	if (mutex->event)
		v_close(mutex->event);
	mutex->event = 0;
	mutex->state = V_MUTEX_UNLOCKED;
	mutex->owner = 0;
}
