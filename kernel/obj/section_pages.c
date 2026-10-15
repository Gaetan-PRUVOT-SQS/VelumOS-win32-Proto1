#include "obj_int.h"
#include "velum/klog.h"
#include "velum/util.h"

static bool	page_is_ours(t_aspace *as, const t_secmap *m, uint64_t i)
{
	t_vminfo	info;

	return (vmm_query(as, m->va + i * PAGE_SIZE, &info)
		&& align_down(info.pa, PAGE_SIZE)
		== section_frame(m->sec, m->offset / PAGE_SIZE + i));
}

bool	sec_pages_mapped(t_aspace *as, const t_secmap *m)
{
	uint64_t	i;

	i = 0;
	while (i < m->len / PAGE_SIZE)
	{
		if (page_is_ours(as, m, i))
			return (true);
		i++;
	}
	return (false);
}

uint64_t	sec_pages_unmap(t_aspace *as, const t_secmap *m)
{
	uint64_t	i;
	uint64_t	left;

	i = 0;
	left = 0;
	while (i < m->len / PAGE_SIZE)
	{
		if (page_is_ours(as, m, i)
			&& vmm_unmap(as, m->va + i * PAGE_SIZE, PAGE_SIZE) < 0)
			left++;
		i++;
	}
	return (left);
}

void	sec_pages_rollback(t_aspace *as, const t_secmap *part)
{
	if (sec_pages_unmap(as, part) == 0)
		return ;
	obj_ref(part->sec);
	klog_warn("objets: section restee mappee, reference gardee");
}

int	secmap_record(t_process *p, t_secmap *m)
{
	int	rc;

	rc = secmap_insert(ht_of(p), m);
	if (rc < 0)
	{
		obj_ref(m->sec);
		secmap_drop(p->aspace, m);
	}
	return (rc);
}
