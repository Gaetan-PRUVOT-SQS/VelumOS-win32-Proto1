#include "sched_int.h"
#include "velum/heap.h"
#include "velum/panic.h"
#include "velum/vmm.h"

void	sched_reap_push_locked(t_cpusched *cs, t_kthread *kt)
{
	kt->reap_next = cs->dead;
	cs->dead = kt;
	if (cs->reaper && cs->reaper != kt)
		sched_unpark_locked(cs, cs->reaper, false);
}

void	sched_exit_current(void)
{
	t_cpusched	*cs;
	t_kthread	*kt;

	cs = sched_cpu();
	kt = sched_kself();
	if (cs->cpu->preempt != 0)
		panic("thread_exit: fil %u sorti avec un verrou tenu", kt->t.tid);
	spin_lock_irqsave(&cs->lock);
	kt->t.state = TS_ZOMBIE;
	sched_pass_locked(cs, SR_EXIT);
}

static t_kthread	*reap_pop(t_cpusched *cs)
{
	t_kthread	*kt;
	uint64_t	flags;

	flags = spin_lock_irqsave(&cs->lock);
	kt = cs->dead;
	if (kt)
	{
		cs->dead = kt->reap_next;
		kt->reap_next = NULL;
	}
	spin_unlock_irqrestore(&cs->lock, flags);
	return (kt);
}

static void	reap_one(t_kthread *kt)
{
	if (!(kt->kflags & KT_STATIC))
	{
		vmm_kstack_free((void *)kt->t.kstack_base, KSTACK_PAGES);
		kfree(kt->t.fpu);
		kt->t.fpu = NULL;
		kt->t.kstack_base = 0;
		kt->t.kstack_top = 0;
	}
	thread_unref(&kt->t);
}

void	sched_reaper_main(void *arg)
{
	t_cpusched	*cs;
	t_kthread	*kt;

	cs = arg;
	while (1)
	{
		kt = reap_pop(cs);
		if (kt)
			reap_one(kt);
		else
			sched_park();
	}
}
