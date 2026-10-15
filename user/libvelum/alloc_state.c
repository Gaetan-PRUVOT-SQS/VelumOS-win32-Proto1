#include "alloc_int.h"

t_alloc	g_alloc;

void	alloc_seed(uint64_t seed)
{
	v_spin_lock(&g_alloc.lock);
	if (!g_alloc.stats.alloc_calls)
		g_alloc.seed = seed;
	v_spin_unlock(&g_alloc.lock);
}

void	alloc_set_fault_hook(t_allocfault hook)
{
	v_spin_lock(&g_alloc.lock);
	g_alloc.fault_hook = hook;
	v_spin_unlock(&g_alloc.lock);
}

void	v_heap_stats(t_vheapstats *out)
{
	if (!out)
		return ;
	v_spin_lock(&g_alloc.lock);
	*out = g_alloc.stats;
	v_spin_unlock(&g_alloc.lock);
}
