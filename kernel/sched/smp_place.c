#include "sched_int.h"

uint32_t	sched_load(const t_cpusched *cs)
{
	return (__atomic_load_n(&cs->rq.count, __ATOMIC_RELAXED)
		+ (uint32_t)__atomic_load_n(&cs->busy, __ATOMIC_RELAXED));
}

uint32_t	sched_least_loaded(t_cpusched *const *tab, uint32_t span)
{
	uint32_t	i;
	uint32_t	best;
	uint32_t	best_load;

	best = span;
	best_load = 0;
	i = 0;
	while (i < span)
	{
		if (tab[i] && (best == span || sched_load(tab[i]) < best_load))
		{
			best = i;
			best_load = sched_load(tab[i]);
		}
		i++;
	}
	return (best);
}

uint32_t	sched_idle_peer(t_cpusched *const *tab, uint32_t span,
				uint32_t except)
{
	uint32_t	i;

	i = 0;
	while (i < span)
	{
		if (i != except && tab[i] && sched_load(tab[i]) == 0)
			return (i);
		i++;
	}
	return (span);
}
