#include "display_int.h"
#include "velum/err.h"
#include "velum/vmm.h"

static t_umap	*slot_for(uint32_t pid)
{
	uint32_t	i;

	i = 0;
	while (i < UMAP_SLOTS)
	{
		if (g_display.umap[i].pid == pid)
			return (&g_display.umap[i]);
		i++;
	}
	i = 0;
	while (i < UMAP_SLOTS)
	{
		if (!g_display.umap[i].pid)
			return (&g_display.umap[i]);
		i++;
	}
	return (&g_display.umap[pid % UMAP_SLOTS]);
}

static bool	still_mapped(t_process *p, const t_umap *u)
{
	t_vminfo	vi;

	if (!vmm_query(p->aspace, (uintptr_t)u->va, &vi))
		return (false);
	return (vi.pa == g_display.phys);
}

static int	release(t_process *p, t_umap *u)
{
	int	rc;

	rc = 0;
	if (u->pid == p->pid && still_mapped(p, u))
		rc = vmm_unmap(p->aspace, (uintptr_t)u->va, (size_t)u->len);
	if (rc == 0)
		u->pid = 0;
	return (rc);
}

static int64_t	install(t_process *p, t_umap *u, uint64_t va, uint64_t len)
{
	t_vmreq	rq;
	int		rc;

	rq.va = (uintptr_t)va;
	rq.pa = g_display.phys;
	rq.len = (size_t)len;
	rq.flags = g_display.map_flags | VM_USER | VM_R | VM_W;
	rc = vmm_map(p->aspace, &rq);
	if (rc < 0)
		return (rc);
	u->pid = p->pid;
	u->gen = g_display.generation;
	u->va = va;
	u->len = len;
	return ((int64_t)va);
}

int64_t	display_user_map(t_process *p, uint64_t hint)
{
	t_umap		*u;
	uint64_t	va;
	uint64_t	len;
	int			rc;

	len = umap_len();
	if (hint && !umap_hint_ok(hint, len))
		return (E_INVAL);
	u = slot_for(p->pid);
	if (u->pid == p->pid && u->gen == g_display.generation
		&& still_mapped(p, u))
		return ((int64_t)u->va);
	rc = release(p, u);
	if (rc == 0)
		rc = umap_place(p->aspace, hint, len, &va);
	if (rc < 0)
		return (rc);
	return (install(p, u, va, len));
}
