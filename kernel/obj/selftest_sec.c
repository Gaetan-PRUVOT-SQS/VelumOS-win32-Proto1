#include "obj_int.h"
#include "velum/libk.h"
#include "velum/util.h"

static int	st_sec_map(t_process *p, t_object *sec, uintptr_t *va)
{
	t_secreq	rq;

	memset(&rq, 0, sizeof(rq));
	rq.prot = PROT_R | PROT_W;
	rq.len = 2 * PAGE_SIZE;
	return (section_map(p, sec, &rq, va));
}

static int	st_sec_procs(t_process *pa, t_process *pb)
{
	memset(pa, 0, sizeof(t_process));
	memset(pb, 0, sizeof(t_process));
	pa->handles = handle_table_create();
	pb->handles = handle_table_create();
	pa->aspace = vmm_aspace_create();
	pb->aspace = vmm_aspace_create();
	pa->mem_limit = 0x1000000;
	pb->mem_limit = 0x1000000;
	if (!pa->handles || !pb->handles || !pa->aspace || !pb->aspace)
		return (E_NOMEM);
	return (0);
}

static void	st_sec_free(t_process *p)
{
	object_process_cleanup(p);
	handle_table_destroy(p->handles);
	if (p->aspace)
		vmm_aspace_destroy(p->aspace);
}

static int	st_sec_check(t_process *pa, t_process *pb, t_object *sec)
{
	uintptr_t	va;
	uintptr_t	vb;
	t_vminfo	ia;
	t_vminfo	ib;
	int			rc;

	rc = st_sec_map(pa, sec, &va);
	if (rc == 0)
		rc = st_sec_map(pb, sec, &vb);
	if (rc < 0)
		return (rc);
	if (!vmm_query(pa->aspace, va + PAGE_SIZE, &ia)
		|| !vmm_query(pb->aspace, vb + PAGE_SIZE, &ib))
		return (E_FAULT);
	if (align_down(ia.pa, PAGE_SIZE) != section_frame(sec, 1)
		|| align_down(ib.pa, PAGE_SIZE) != section_frame(sec, 1))
		return (E_PROTO);
	return (0);
}

int	st_section(void)
{
	t_process	pa;
	t_process	pb;
	t_object	*sec;
	uint64_t	user0;
	int			rc;

	user0 = st_pmm_user();
	rc = st_sec_procs(&pa, &pb);
	if (rc == 0)
		rc = section_create(&pa, 2 * PAGE_SIZE, PROT_R | PROT_W, &sec);
	if (rc == 0)
	{
		rc = st_sec_check(&pa, &pb, sec);
		obj_unref(sec);
	}
	st_sec_free(&pa);
	st_sec_free(&pb);
	if (rc == 0 && st_pmm_user() != user0)
		rc = E_IO;
	return (rc);
}
