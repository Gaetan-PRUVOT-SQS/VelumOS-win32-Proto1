#include "sched_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/panic.h"
#include "velum/timer.h"

static void	wq_check_sleep(t_spinlock *held)
{
	uint32_t	want;
	uint32_t	depth;

	if (!SCHED_DEBUG || !sched_lockstack())
		return ;
	want = 0;
	if (held)
		want = 1;
	depth = lockdep_depth();
	if (depth != want)
		panic("waitq: attente avec %u verrou(s) tenu(s) au lieu de %u "
			"(appelant %p)", depth, want, __builtin_return_address(0));
}

static int	wq_park_loop(t_waitq *wq, t_kthread *kt, bool cancelable)
{
	int32_t	r;

	while (1)
	{
		sched_park();
		spin_lock(&wq->lock);
		if (kt->wq != wq)
		{
			r = kt->t.wait_result;
			spin_unlock(&wq->lock);
			return (r);
		}
		r = __atomic_load_n(&kt->reason, __ATOMIC_SEQ_CST);
		if (r != 0 && (cancelable || r != E_CANCELED))
		{
			waitq_list_remove(wq, kt);
			kt->wq = NULL;
			spin_unlock(&wq->lock);
			return (r);
		}
		spin_unlock(&wq->lock);
	}
}

static int	wq_sleep(t_waitq *wq, t_spinlock *held, uint64_t deadline,
				bool cancelable)
{
	t_kthread	*kt;
	int			rc;

	kt = sched_kself();
	spin_unlock(&wq->lock);
	if (held && held != &wq->lock)
		spin_unlock(held);
	wq_arm(kt, deadline);
	rc = wq_park_loop(wq, kt, cancelable);
	wq_disarm(kt);
	if (held)
		spin_lock(held);
	return (rc);
}

int	wq_block(t_waitq *wq, t_spinlock *held, uint64_t deadline,
		bool cancelable)
{
	uint64_t	flags;
	int			rc;

	wq_check_sleep(held);
	flags = irq_save();
	if (held != &wq->lock)
		spin_lock(&wq->lock);
	if (!wq_enter(wq, sched_kself(), cancelable))
	{
		if (held != &wq->lock)
			spin_unlock(&wq->lock);
		irq_restore(flags);
		return (E_CANCELED);
	}
	rc = wq_sleep(wq, held, deadline, cancelable);
	irq_restore(flags);
	return (rc);
}

int	waitq_wait(t_waitq *wq, t_spinlock *held, uint64_t timeout_ns)
{
	t_kthread	*kt;
	uint64_t	now;
	uint64_t	deadline;

	kt = sched_kself();
	if (__atomic_load_n(&kt->canceled, __ATOMIC_SEQ_CST))
		return (E_CANCELED);
	if (timeout_ns == 0)
		return (E_TIMEOUT);
	deadline = TIMEOUT_NONE;
	if (timeout_ns != TIMEOUT_NONE)
	{
		now = time_now_ns();
		if (now <= TIMEOUT_NONE - 1 - timeout_ns)
			deadline = now + timeout_ns;
	}
	return (wq_block(wq, held, deadline, true));
}
