#include "fake.h"

static t_fkmap	*fk_map_find(void *as, uint64_t va)
{
	uint32_t	i;

	i = 0;
	while (i < FK_MAPS)
	{
		if (g_fk.maps[i].used && g_fk.maps[i].as == as
			&& g_fk.maps[i].va == va)
			return (&g_fk.maps[i]);
		i++;
	}
	return (NULL);
}

static int	fk_map_add(void *as, uint64_t va, uint64_t pa, uint32_t flags)
{
	uint32_t	i;

	i = 0;
	while (i < FK_MAPS && g_fk.maps[i].used)
		i++;
	if (i == FK_MAPS)
		return (E_NOMEM);
	g_fk.maps[i].used = 1;
	g_fk.maps[i].as = as;
	g_fk.maps[i].va = va;
	g_fk.maps[i].pa = pa;
	g_fk.maps[i].flags = flags;
	return (0);
}

static bool	fk_range_busy(void *as, uint64_t va, uint64_t len)
{
	uint64_t	off;

	off = 0;
	while (off < len)
	{
		if (fk_map_find(as, va + off))
			return (true);
		off += PAGE_SIZE;
	}
	return (false);
}

int	vmm_map(t_aspace *as, const t_vmreq *rq)
{
	uint64_t	off;

	if (g_fk.vmm_fail > 0)
	{
		g_fk.vmm_fail--;
		if (g_fk.vmm_fail == 0)
			return (fk_vmm_refuse());
	}
	if (rq->len == 0 || rq->len % PAGE_SIZE
		|| ((rq->flags & VM_W) && (rq->flags & VM_X)))
		return (E_INVAL);
	if (fk_range_busy(as, rq->va, rq->len))
		return (E_EXIST);
	off = 0;
	while (off < rq->len)
	{
		if (fk_map_add(as, rq->va + off, rq->pa + off, rq->flags) < 0)
			return (E_NOMEM);
		off += PAGE_SIZE;
	}
	return (0);
}

int	vmm_unmap(t_aspace *as, uintptr_t va, size_t len)
{
	uint32_t	i;

	if (g_fk.unmap_fail)
		return (E_ACCES);
	i = 0;
	while (i < FK_MAPS)
	{
		if (g_fk.maps[i].used && g_fk.maps[i].as == as
			&& g_fk.maps[i].va >= va && g_fk.maps[i].va < va + len)
			g_fk.maps[i].used = 0;
		i++;
	}
	return (0);
}
