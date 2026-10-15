#include "velum/err.h"
#include "velum/vobj.h"
#include "velum/vsync.h"
#include "velum/vtime.h"
#include "velum/vtls.h"

static int64_t	mutex_event(t_vmutex *m)
{
	int64_t		h;
	uint32_t	expected;

	expected = __atomic_load_n(&m->event, __ATOMIC_ACQUIRE);
	if (expected)
		return (expected);
	h = v_event_create(0, 0);
	if (h < 0)
		return (h);
	expected = 0;
	if (!__atomic_compare_exchange_n(&m->event, &expected, (uint32_t)h, 0,
			__ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
	{
		v_close((t_handle)h);
		return (expected);
	}
	return (h);
}

static void	mutex_block(t_vmutex *m)
{
	int64_t	h;

	h = mutex_event(m);
	if (h < 0 || v_wait((t_handle)h, TIMEOUT_INF) < 0)
		v_yield();
}

void	v_mutex_init(t_vmutex *mutex)
{
	mutex->state = V_MUTEX_UNLOCKED;
	mutex->owner = 0;
	mutex->event = 0;
	mutex->reserved = 0;
}

int	v_mutex_lock(t_vmutex *mutex)
{
	uint32_t	tid;
	uint32_t	expected;

	tid = v_tcb()->tid;
	if (tid && __atomic_load_n(&mutex->owner, __ATOMIC_RELAXED) == tid)
		return (E_DEADLK);
	expected = V_MUTEX_UNLOCKED;
	if (!__atomic_compare_exchange_n(&mutex->state, &expected,
			V_MUTEX_LOCKED, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
	{
		while (__atomic_exchange_n(&mutex->state, V_MUTEX_CONTENDED,
				__ATOMIC_ACQUIRE) != V_MUTEX_UNLOCKED)
			mutex_block(mutex);
	}
	__atomic_store_n(&mutex->owner, tid, __ATOMIC_RELAXED);
	return (0);
}

int	v_mutex_trylock(t_vmutex *mutex)
{
	uint32_t	expected;

	expected = V_MUTEX_UNLOCKED;
	if (!__atomic_compare_exchange_n(&mutex->state, &expected,
			V_MUTEX_LOCKED, 0, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
		return (E_BUSY);
	__atomic_store_n(&mutex->owner, v_tcb()->tid, __ATOMIC_RELAXED);
	return (0);
}
