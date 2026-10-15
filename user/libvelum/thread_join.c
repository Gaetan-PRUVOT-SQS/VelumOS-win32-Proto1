#include "stdlib.h"
#include "thread_int.h"
#include "velum/err.h"
#include "velum/vobj.h"

void	thread_cleanup(t_vthread *thread)
{
	v_event_destroy(&thread->done);
	if (thread->handle)
		v_close(thread->handle);
	free(thread);
}

int	v_thread_join(t_vthread *thread, void **result)
{
	int	rc;

	if (!thread)
		return (E_INVAL);
	if (thread == v_thread_self())
		return (E_DEADLK);
	if (__atomic_exchange_n(&thread->joining, 1, __ATOMIC_SEQ_CST))
		return (E_INVAL);
	rc = v_event_wait(&thread->done, TIMEOUT_INF);
	if (rc < 0)
	{
		__atomic_store_n(&thread->joining, 0, __ATOMIC_SEQ_CST);
		return (rc);
	}
	if (result)
		*result = thread->result;
	thread_cleanup(thread);
	return (0);
}

int	v_thread_detach(t_vthread *thread)
{
	uint32_t	expected;

	if (!thread)
		return (E_INVAL);
	if (__atomic_exchange_n(&thread->joining, 1, __ATOMIC_SEQ_CST))
		return (E_INVAL);
	expected = V_THREAD_RUNNING;
	if (__atomic_compare_exchange_n(&thread->state, &expected,
			V_THREAD_DETACHED, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		return (0);
	v_event_wait(&thread->done, TIMEOUT_INF);
	thread_cleanup(thread);
	return (0);
}
