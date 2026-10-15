#include "fake.h"

void	fk_vmq_fire(uint64_t page, int pre)
{
	t_fkvmq	fn;

	if (!g_fk.vmq_hook || g_fk.vmq_va != page || g_fk.vmq_pre != pre)
		return ;
	fn = g_fk.vmq_hook;
	g_fk.vmq_hook = NULL;
	fn();
}

void	fk_vmq_arm(t_fkvmq fn, uint64_t va, int pre)
{
	g_fk.vmq_hook = fn;
	g_fk.vmq_va = va;
	g_fk.vmq_pre = pre;
}

bool	vmm_query(t_aspace *as, uintptr_t va, t_vminfo *out)
{
	uint32_t	i;
	uint64_t	page;
	bool		found;

	page = va & ~(uint64_t)(PAGE_SIZE - 1);
	fk_vmq_fire(page, 1);
	found = false;
	i = 0;
	while (!found && i < FK_MAPS)
	{
		if (g_fk.maps[i].used && g_fk.maps[i].as == (void *)as
			&& g_fk.maps[i].va == page)
		{
			out->pa = g_fk.maps[i].pa;
			out->flags = g_fk.maps[i].flags;
			found = true;
		}
		i++;
	}
	fk_vmq_fire(page, 0);
	return (found);
}

uint32_t	fk_count_maps(void *as)
{
	uint32_t	i;
	uint32_t	n;

	i = 0;
	n = 0;
	while (i < FK_MAPS)
	{
		if (g_fk.maps[i].used && (!as || g_fk.maps[i].as == as))
			n++;
		i++;
	}
	return (n);
}

uint64_t	fk_uptr(const void *p)
{
	return ((uint64_t)(uintptr_t)p);
}
