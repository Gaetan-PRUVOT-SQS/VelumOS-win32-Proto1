#include "sched_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/libk.h"

static t_ranktab	g_ranks = {{{"mutex", RANK_MUTEX}, {"event", RANK_MUTEX},
{"sem", RANK_MUTEX}, {"waitq", RANK_WAITQ}, {"runq", RANK_RUNQ}}, 5, 0};

uint32_t	lockdep_rank_of(const char *name)
{
	uint32_t	n;
	uint32_t	i;

	if (!name)
		return (0);
	n = __atomic_load_n(&g_ranks.count, __ATOMIC_ACQUIRE);
	i = 0;
	while (i < n)
	{
		if (!strcmp(g_ranks.ent[i].name, name))
			return (g_ranks.ent[i].rank);
		i++;
	}
	return (0);
}

static int	rank_insert(const char *name, uint32_t rank)
{
	uint32_t	known;
	uint32_t	n;

	known = lockdep_rank_of(name);
	if (known == rank)
		return (0);
	if (known != 0)
		return (E_EXIST);
	n = g_ranks.count;
	if (n >= RANK_SLOTS)
		return (E_NOSPC);
	g_ranks.ent[n].name = name;
	g_ranks.ent[n].rank = rank;
	__atomic_store_n(&g_ranks.count, n + 1, __ATOMIC_RELEASE);
	return (0);
}

int	spin_rank_register(const char *name, uint32_t rank)
{
	uint64_t	flags;
	int			rc;

	if (!name || !name[0] || rank == 0 || rank > 1000)
		return (E_INVAL);
	flags = irq_save();
	while (__atomic_exchange_n(&g_ranks.busy, 1, __ATOMIC_ACQUIRE))
		cpu_relax();
	rc = rank_insert(name, rank);
	__atomic_store_n(&g_ranks.busy, 0, __ATOMIC_RELEASE);
	irq_restore(flags);
	return (rc);
}
