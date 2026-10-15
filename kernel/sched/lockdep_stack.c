#include "sched_int.h"
#include "velum/irqflags.h"
#include "velum/panic.h"

static void	lockdep_check(t_lockstack *ls, t_spinlock *l, uint32_t rank,
				void *caller)
{
	uint32_t	i;
	t_held		*h;

	i = 0;
	while (i < ls->depth)
	{
		h = &ls->held[i];
		if (h->rank != 0 && h->rank >= rank)
			panic("lockdep: '%s' (rang %u) pris sous '%s' (rang %u), "
				"appelant %p", l->name, rank, h->lock->name, h->rank, caller);
		i++;
	}
}

void	lockdep_acquire(t_spinlock *l, void *caller, bool check)
{
	t_lockstack	*ls;
	uint32_t	rank;
	uint64_t	flags;

	ls = sched_lockstack();
	if (!ls)
		return ;
	rank = lockdep_rank_of(l->name);
	flags = irq_save();
	if (check && rank != 0)
		lockdep_check(ls, l, rank, caller);
	if (ls->depth >= LOCKDEP_DEPTH)
		panic("lockdep: plus de %d verrous tenus ('%s', appelant %p)",
			LOCKDEP_DEPTH, l->name, caller);
	ls->held[ls->depth].lock = l;
	ls->held[ls->depth].caller = caller;
	ls->held[ls->depth].rank = rank;
	ls->depth++;
	irq_restore(flags);
}

static void	lockdep_drop(t_lockstack *ls, uint32_t i)
{
	while (i + 1 < ls->depth)
	{
		ls->held[i] = ls->held[i + 1];
		i++;
	}
	ls->depth--;
}

void	lockdep_release(t_spinlock *l)
{
	t_lockstack	*ls;
	uint32_t	i;
	uint64_t	flags;

	ls = sched_lockstack();
	if (!ls)
		return ;
	flags = irq_save();
	i = ls->depth;
	while (i > 0)
	{
		i--;
		if (ls->held[i].lock == l)
		{
			lockdep_drop(ls, i);
			break ;
		}
	}
	irq_restore(flags);
}

uint32_t	lockdep_depth(void)
{
	t_lockstack	*ls;

	ls = sched_lockstack();
	if (!ls)
		return (0);
	return (ls->depth);
}
