#include "velum/err.h"
#include "velum/libk.h"
#include "vmm_int.h"
#include "vmm_weak.h"

static uintptr_t	pick_base(void)
{
	uint64_t	slots;
	uint64_t	r;

	slots = (ASLR_HI - ASLR_LO) / ALLOC_GRAN;
	if (krandom_below)
		r = krandom_below(slots);
	else
		r = __atomic_fetch_add(&g_vmm.cursor, 1, __ATOMIC_RELAXED) % slots;
	return (ASLR_LO + r * ALLOC_GRAN);
}

static int64_t	place(t_aspace *as, const t_vareq *rq)
{
	uintptr_t	va;
	int			rc;

	if (rq->limit && vmm_pages_used(as) * PAGE_SIZE + rq->len > rq->limit)
		return (E_NOMEM);
	va = rq->hint;
	if (!va)
		va = vmm_find_locked(as, rq->len, pick_base(), USER_TOP);
	if (!va && !rq->hint)
		va = vmm_find_locked(as, rq->len, USER_MIN, USER_TOP);
	if (!va)
		return (E_NOMEM);
	rc = vmm_alloc_locked(as, va, rq->len, rq->fl);
	if (rc < 0)
		return (rc);
	return ((int64_t)va);
}

int64_t	vmm_sys_valloc(const t_sysargs *a)
{
	t_process	*p;
	t_aspace	*as;
	t_vareq		rq;
	uint64_t	irq;
	int64_t		rc;

	as = vmm_sys_caller(&p);
	if (!as)
		return (E_PERM);
	if (!a->a[1] || a->a[1] > USER_TOP - USER_MIN
		|| vmm_sys_prot(a->a[2], &rq.fl) < 0)
		return (E_INVAL);
	rq.hint = a->a[0];
	rq.len = align_up(a->a[1], PAGE_SIZE);
	rq.limit = p->mem_limit;
	irq = vmm_lock(as);
	rc = place(as, &rq);
	vmm_unlock(as, irq);
	return (rc);
}

static void	query_fill(t_aspace *as, uintptr_t va, t_vquery *q)
{
	t_vmregion	*r;
	uintptr_t	page;

	page = align_down(va, PAGE_SIZE);
	memset(q, 0, sizeof(*q));
	r = vmm_reg_from(as, page);
	q->size = USER_TOP - page;
	if (r && r->start <= page)
	{
		if (r->flags & VM_R)
			q->prot = PROT_R;
		if (r->flags & VM_W)
			q->prot |= PROT_W;
		if (r->flags & VM_X)
			q->prot |= PROT_X;
		q->size = r->end - page;
	}
	else if (r)
		q->size = r->start - page;
}

int64_t	vmm_sys_vquery(const t_sysargs *a)
{
	t_process	*p;
	t_aspace	*as;
	t_vquery	q;
	uint64_t	irq;

	as = vmm_sys_caller(&p);
	if (!as)
		return (E_PERM);
	if (a->a[0] < USER_MIN || a->a[0] >= USER_TOP)
		return (E_INVAL);
	irq = vmm_lock(as);
	query_fill(as, a->a[0], &q);
	vmm_unlock(as, irq);
	return (copy_to_user(a->a[1], &q, sizeof(q)));
}
