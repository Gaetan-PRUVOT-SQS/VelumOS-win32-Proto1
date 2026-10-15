#include "vmm_int.h"

t_hspan	*vmm_hhdm_find(uint64_t phys, uint64_t len)
{
	uint64_t	end;
	uint32_t	i;

	if (__builtin_add_overflow(phys, len, &end))
		return (NULL);
	i = 0;
	while (i < g_vmm.nspan)
	{
		if (phys >= g_vmm.hspan[i].base && end <= g_vmm.hspan[i].end)
			return (&g_vmm.hspan[i]);
		i++;
	}
	return (NULL);
}

bool	vmm_hhdm_covers(uint64_t phys, uint64_t len)
{
	return (vmm_hhdm_find(phys, len) != NULL);
}

uint64_t	vmm_hhdm_top(void)
{
	if (!g_vmm.nspan)
		return (0);
	return (g_vmm.hspan[g_vmm.nspan - 1].end);
}

int	vmm_hhdm_map(t_aspace *as, uint64_t hhdm)
{
	t_vmreq		rq;
	uint32_t	i;
	int			rc;

	i = 0;
	while (i < g_vmm.nspan)
	{
		rq.va = hhdm + g_vmm.hspan[i].base;
		rq.pa = g_vmm.hspan[i].base;
		rq.len = g_vmm.hspan[i].end - g_vmm.hspan[i].base;
		rq.flags = 0;
		rc = vmm_map_span(as, &rq, g_vmm.hspan[i].bits);
		if (rc < 0)
			return (rc);
		i++;
	}
	return (0);
}
