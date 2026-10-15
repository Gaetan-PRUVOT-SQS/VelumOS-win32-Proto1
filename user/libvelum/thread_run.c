#include "stdlib.h"
#include "thread_int.h"
#include "velum/vproc.h"

static _Noreturn void	thread_finish(t_vthread *t)
{
	uint32_t	expected;

	expected = V_THREAD_RUNNING;
	if (__atomic_compare_exchange_n(&t->state, &expected, V_THREAD_FINISHED,
			0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
		v_event_set(&t->done);
	else
		thread_cleanup(t);
	v_thread_exit_raw(0);
}

_Noreturn void	__velum_thread_main(t_vthread *thread)
{
	t_vtcb	*tcb;

	tcb = &thread->tcb;
	if (v_set_fsbase((uint64_t)(uintptr_t)tcb) < 0)
	{
		thread->result = NULL;
		thread_finish(thread);
	}
	thread->tcb.tid = (uint32_t)v_gettid();
	thread->result = thread->fn(thread->arg);
	thread_finish(thread);
}

_Noreturn void	v_thread_exit(void *result)
{
	t_vthread	*t;

	t = v_thread_self();
	if (!t)
		exit((int)(intptr_t)result);
	t->result = result;
	thread_finish(t);
}

t_vthread	*v_thread_self(void)
{
	return (v_tcb()->thread);
}
