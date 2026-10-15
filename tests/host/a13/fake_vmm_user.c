#include "fakes.h"
#include "velum/err.h"
#include "velum/vmm.h"

static t_fmap	*find(t_aspace *as, uint64_t va)
{
	int	i;

	i = 0;
	while (i < FAKE_MAP)
	{
		if (g_fvmm.map[i].live && g_fvmm.map[i].as == as
			&& va >= g_fvmm.map[i].va
			&& va < g_fvmm.map[i].va + g_fvmm.map[i].len)
			return (&g_fvmm.map[i]);
		i++;
	}
	return (NULL);
}

t_fmap	*fake_vmm_overlap(t_aspace *as, uint64_t va, uint64_t len)
{
	int	i;

	i = 0;
	while (i < FAKE_MAP)
	{
		if (g_fvmm.map[i].live && g_fvmm.map[i].as == as
			&& va < g_fvmm.map[i].va + g_fvmm.map[i].len
			&& g_fvmm.map[i].va < va + len)
			return (&g_fvmm.map[i]);
		i++;
	}
	return (NULL);
}

int	vmm_map(t_aspace *as, const t_vmreq *rq)
{
	uint32_t	call;
	t_fmap		*m;
	int			i;

	call = g_fvmm.map_calls++;
	if (call < 32 && ((g_fvmm.fail_map_mask >> call) & 1))
		return (E_NOMEM);
	if (fake_vmm_overlap(as, rq->va, rq->len))
		return (E_EXIST);
	i = 0;
	while (i < FAKE_MAP - 1 && g_fvmm.map[i].live)
		i++;
	m = &g_fvmm.map[i];
	if (m->live)
		return (E_NOMEM);
	m->as = as;
	m->va = rq->va;
	m->pa = rq->pa;
	m->len = rq->len;
	m->flags = rq->flags;
	m->live = true;
	g_fvmm.maps++;
	return (E_OK);
}

int	vmm_unmap(t_aspace *as, uintptr_t va, size_t len)
{
	t_fmap	*m;

	m = find(as, va);
	if (!m || m->va != va || m->len != len)
		return (E_INVAL);
	m->live = false;
	g_fvmm.unmaps++;
	return (E_OK);
}

bool	vmm_query(t_aspace *as, uintptr_t va, t_vminfo *out)
{
	t_fmap	*m;

	m = find(as, va);
	if (!m)
		return (false);
	out->pa = m->pa + (va - m->va);
	out->flags = m->flags;
	return (true);
}
