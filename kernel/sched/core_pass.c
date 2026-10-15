#include "sched_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"
#include "velum/timer.h"

void	sched_resched(int why)
{
	t_cpusched	*cs;
	uint64_t	flags;

	cs = sched_cpu();
	if (cs->cpu->preempt != 0)
	{
		__atomic_store_n(&cs->need_resched, 1, __ATOMIC_RELEASE);
		return ;
	}
	flags = spin_lock_irqsave(&cs->lock);
	sched_pass_locked(cs, why);
	spin_unlock(&cs->lock);
	sched_finish(cs);
	irq_restore(flags);
}

static void	sched_switch_check(t_cpusched *cs, t_kthread *prev)
{
	if (cs->cpu->preempt != 1)
		panic("sched: bascule depuis le fil %u avec %u section(s) sans "
			"préemption (verrou tenu ?)", prev->t.tid, cs->cpu->preempt - 1);
	if (SCHED_DEBUG && cs->locks.depth != 1)
		panic("sched: bascule depuis le fil %u avec %u verrou(s) tenu(s)",
			prev->t.tid, cs->locks.depth);
}

void	sched_pass_locked(t_cpusched *cs, int why)
{
	t_kthread	*prev;
	t_kthread	*next;
	uint64_t	now;

	prev = (t_kthread *)cs->cpu->current;
	now = time_now_ns();
	policy_account(prev, &cs->slice_start, now);
	prev->ready_ns = now;
	if (prev->t.state == TS_ZOMBIE)
		sched_reap_push_locked(cs, prev);
	next = policy_pick(&cs->rq, prev, why, cs->idle);
	__atomic_store_n(&cs->need_resched, 0, __ATOMIC_RELEASE);
	if (next->t.quantum_left == 0)
		next->t.quantum_left = (uint32_t)cs->quantum_ns;
	next->t.state = TS_RUNNING;
	if (next == prev)
		return ;
	sched_switch_check(cs, prev);
	cs->switches++;
	arch_switch_to(cs, prev, next);
}

void	sched_finish(t_cpusched *cs)
{
	t_kthread	*cur;
	int64_t		id;

	cur = (t_kthread *)cs->cpu->current;
	if ((cur->kflags & KT_IDLE) || cs->rq.count == 0
		|| __atomic_load_n(&cs->qtimer_pending, __ATOMIC_ACQUIRE))
		return ;
	__atomic_store_n(&cs->qtimer_pending, 1, __ATOMIC_RELEASE);
	id = timer_arm(cs->slice_start + cur->t.quantum_left, sched_quantum_cb,
			cs);
	if (id <= 0)
		__atomic_store_n(&cs->qtimer_pending, 0, __ATOMIC_RELEASE);
	else
		cs->qtimer_id = id;
}
