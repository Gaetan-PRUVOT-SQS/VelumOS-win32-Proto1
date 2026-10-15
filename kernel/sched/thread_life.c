#include "sched_int.h"
#include "velum/err.h"
#include "velum/heap.h"
#include "velum/panic.h"

extern void	obj_signal(struct s_object *obj) __attribute__((weak));

_Noreturn void	thread_exit(int code)
{
	t_kthread	*kt;
	uint64_t	flags;

	kt = sched_kself();
	if (!__atomic_load_n(&g_sched.ready, __ATOMIC_ACQUIRE)
		|| (kt->kflags & KT_IDLE))
		panic("thread_exit: le fil %u ne peut pas sortir", kt->t.tid);
	flags = spin_lock_irqsave(&kt->t.joiners.lock);
	kt->t.exit_code = code;
	__atomic_store_n(&kt->exited, 1, __ATOMIC_RELEASE);
	wq_wake_all_locked(&kt->t.joiners, 0);
	spin_unlock_irqrestore(&kt->t.joiners.lock, flags);
	if (kt->t.obj && obj_signal)
		obj_signal(kt->t.obj);
	sched_exit_current();
	panic("thread_exit: le fil %u est revenu de la mort", kt->t.tid);
}

int	thread_join(t_thread *t, uint64_t timeout_ns)
{
	t_kthread	*kt;
	uint64_t	flags;
	int			rc;

	if (!t)
		return (E_INVAL);
	kt = (t_kthread *)t;
	if (kt == sched_kself())
		return (E_DEADLK);
	flags = spin_lock_irqsave(&t->joiners.lock);
	rc = 0;
	while (rc == 0 && !__atomic_load_n(&kt->exited, __ATOMIC_ACQUIRE))
		rc = waitq_wait(&t->joiners, &t->joiners.lock, timeout_ns);
	spin_unlock_irqrestore(&t->joiners.lock, flags);
	return (rc);
}

void	thread_ref(t_thread *t)
{
	if (t)
		__atomic_add_fetch(&t->refs, 1, __ATOMIC_ACQ_REL);
}

void	thread_unref(t_thread *t)
{
	uint32_t	left;

	if (!t)
		return ;
	left = __atomic_sub_fetch(&t->refs, 1, __ATOMIC_ACQ_REL);
	if (left == UINT32_MAX)
		panic("thread_unref: références négatives sur le fil %u", t->tid);
	if (left == 0)
		thread_free((t_kthread *)t);
}

void	thread_free(t_kthread *kt)
{
	if (kt->kflags & KT_STATIC)
		return ;
	if (kt->t.kstack_base != 0 || kt->t.state != TS_ZOMBIE)
		panic("thread: libération du fil %u encore vivant", kt->t.tid);
	kfree(kt);
}
