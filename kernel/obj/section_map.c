#include "obj_int.h"
#include "velum/heap.h"
#include "velum/util.h"

static int	sec_req_ok(const t_section *s, const t_secreq *rq)
{
	uint64_t	end;

	if (!sec_args_ok(rq->len, rq->prot))
		return (E_INVAL);
	if ((rq->prot & ~s->maxprot) != 0)
		return (E_PERM);
	if (!is_aligned(rq->len, PAGE_SIZE) || !is_aligned(rq->offset, PAGE_SIZE)
		|| !is_aligned(rq->hint, PAGE_SIZE))
		return (E_INVAL);
	if (__builtin_add_overflow(rq->offset, rq->len, &end)
		|| end > s->npages * PAGE_SIZE)
		return (E_INVAL);
	return (0);
}

static uint32_t	sec_vmflags(uint32_t prot)
{
	uint32_t	fl;

	fl = VM_USER | VM_SHARED | VM_R;
	if (prot & PROT_W)
		fl |= VM_W;
	if (prot & PROT_X)
		fl |= VM_X;
	return (fl);
}

static int	sec_map_pages(t_aspace *as, t_object *sec, uintptr_t va,
		const t_secreq *rq)
{
	t_vmreq		vr;
	t_secmap	part;
	uint64_t	i;
	int			rc;

	i = 0;
	rc = 0;
	while (rc == 0 && i < rq->len / PAGE_SIZE)
	{
		vr.va = va + i * PAGE_SIZE;
		vr.pa = section_frame(sec, rq->offset / PAGE_SIZE + i);
		vr.len = PAGE_SIZE;
		vr.flags = sec_vmflags(rq->prot);
		rc = vmm_map(as, &vr);
		if (rc == 0)
			i++;
	}
	if (rc == 0 || i == 0)
		return (rc);
	part = (t_secmap){NULL, va, i * PAGE_SIZE, rq->offset, sec};
	sec_pages_rollback(as, &part);
	return (rc);
}

static int	sec_place(t_aspace *as, t_object *s, const t_secreq *rq,
		uintptr_t *va)
{
	int			rc;
	uint32_t	tries;

	rc = E_EXIST;
	if (rq->hint >= USER_MIN && rq->hint <= USER_TOP - rq->len)
	{
		*va = rq->hint;
		rc = sec_map_pages(as, s, *va, rq);
	}
	tries = 0;
	while (rc == E_EXIST && tries < SEC_MAP_TRIES)
	{
		*va = vmm_find_free(as, rq->len, USER_MIN, USER_TOP);
		rc = E_NOMEM;
		if (*va)
			rc = sec_map_pages(as, s, *va, rq);
		tries++;
	}
	return (rc);
}

int	section_map(t_process *p, t_object *s, t_secreq *r, uintptr_t *v)
{
	t_secmap	*m;
	int			rc;

	if (!ht_of(p) || !p->aspace || !s || !r || !v || section_frame(s, 0) == 0)
		return (E_INVAL);
	rc = sec_req_ok(s->impl, r);
	if (rc < 0)
		return (rc);
	m = kmalloc_tag(sizeof(t_secmap), HEAP_OBJECT);
	if (!m)
		return (E_NOMEM);
	mutex_lock(&ht_of(p)->maplock);
	rc = sec_place(p->aspace, s, r, v);
	if (rc < 0)
		kfree(m);
	else
	{
		*m = (t_secmap){NULL, *v, r->len, r->offset, s};
		rc = secmap_record(p, m);
	}
	mutex_unlock(&ht_of(p)->maplock);
	return (rc);
}
