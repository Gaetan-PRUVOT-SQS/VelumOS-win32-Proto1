#include "obj_int.h"
#include "velum/util.h"

bool	sec_args_ok(uint64_t size, uint32_t prot)
{
	if (size == 0 || prot == 0 || (prot & ~PROT_ALL))
		return (false);
	return (!((prot & PROT_W) && (prot & PROT_X)));
}

int	sec_charge(t_process *p, t_section *s)
{
	t_htab		*ht;
	uint64_t	used;
	int			rc;

	ht = ht_of(p);
	used = 0;
	if (p->aspace)
		used = vmm_pages_used(p->aspace) * PAGE_SIZE;
	rc = acct_charge(ht->acct, used, p->mem_limit, s->npages * PAGE_SIZE);
	if (rc < 0)
		return (rc);
	acct_ref(ht->acct);
	s->acct = ht->acct;
	return (0);
}

uint64_t	section_frame(t_object *sec, uint64_t page)
{
	t_section	*s;

	if (!sec || sec->type != OBJ_SECTION
		|| sec->ops->destroy != section_destroy)
		return (0);
	s = sec->impl;
	if (page >= s->npages)
		return (0);
	return (s->frames[page]);
}
