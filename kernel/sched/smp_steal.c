#include "sched_int.h"
#include "velum/irqflags.h"

static bool	steal_permis(const t_kthread *kt)
{
	return (!(kt->kflags & (KT_PINNED | KT_IDLE | KT_STATIC)));
}

t_kthread	*runq_steal(t_runq *rq)
{
	int32_t		p;
	t_kthread	*kt;

	p = PRIO_MAX;
	while (p >= 0)
	{
		kt = rq->tail[p];
		while (kt && !steal_permis(kt))
			kt = (t_kthread *)kt->t.prev;
		if (kt)
		{
			runq_remove(rq, kt);
			return (kt);
		}
		p--;
	}
	return (NULL);
}

uint32_t	sched_victim(t_cpusched *const *tab, uint32_t span, uint32_t self,
				uint32_t gap)
{
	uint32_t	i;
	uint32_t	best;
	uint32_t	best_load;
	uint32_t	load;
	uint32_t	mine;

	if (self >= span || !tab[self])
		return (span);
	if (gap == 0)
		gap = 1;
	mine = __atomic_load_n(&tab[self]->rq.count, __ATOMIC_RELAXED);
	best = span;
	best_load = mine + gap - 1;
	i = 0;
	while (i < span)
	{
		if (i != self && tab[i])
		{
			load = __atomic_load_n(&tab[i]->rq.count, __ATOMIC_RELAXED);
			if (load > best_load)
			{
				best = i;
				best_load = load;
			}
		}
		i++;
	}
	return (best);
}

static t_kthread	*pull_take(t_cpusched *victim, t_cpusched *self)
{
	t_kthread	*kt;

	if (!spin_trylock(&victim->lock))
		return (NULL);
	kt = runq_steal(&victim->rq);
	if (kt)
		__atomic_store_n(&kt->home, self->id, __ATOMIC_RELEASE);
	spin_unlock(&victim->lock);
	return (kt);
}

bool	sched_pull(t_cpusched *const *tab, uint32_t span, t_cpusched *self)
{
	uint32_t	v;
	uint64_t	flags;
	t_kthread	*kt;

	v = sched_victim(tab, span, self->id, 1);
	if (v >= span)
		return (false);
	flags = spin_lock_irqsave(&self->lock);
	kt = pull_take(tab[v], self);
	if (kt)
	{
		runq_push(&self->rq, kt, false);
		self->steals++;
		__atomic_store_n(&self->need_resched, 1, __ATOMIC_RELEASE);
	}
	spin_unlock_irqrestore(&self->lock, flags);
	return (kt != NULL);
}
