#include "obj_int.h"

static uint32_t	prot_rights(uint64_t prot)
{
	uint32_t	r;

	r = 0;
	if (prot & PROT_R)
		r |= HR_MAP_R;
	if (prot & PROT_W)
		r |= HR_MAP_W;
	if (prot & PROT_X)
		r |= HR_MAP_X;
	return (r);
}

int64_t	sys_section_create(const t_sysargs *a)
{
	t_object	*o;
	int			rc;

	if (a->a[1] & ~(uint64_t)PROT_ALL)
		return (E_INVAL);
	rc = section_create(sys_proc(), a->a[0], (uint32_t)a->a[1], &o);
	if (rc < 0)
		return (rc);
	return (sys_install(o, RIGHTS_SECTION | prot_rights(a->a[1])));
}

int64_t	sys_section_map(const t_sysargs *a)
{
	t_hget		g;
	t_secreq	rq;
	uintptr_t	va;
	int			rc;

	if (a->a[2] & ~(uint64_t)PROT_ALL)
		return (E_INVAL);
	rc = sys_handle(a->a[0], OBJ_SECTION, prot_rights(a->a[2]), &g);
	if (rc < 0)
		return (rc);
	rq.hint = a->a[1];
	rq.prot = (uint32_t)a->a[2];
	rq.offset = a->a[3];
	rq.len = a->a[4];
	rq.pad = 0;
	rc = section_map(sys_proc(), g.obj, &rq, &va);
	obj_unref(g.obj);
	if (rc < 0)
		return (rc);
	return ((int64_t)va);
}
