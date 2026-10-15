#include "velum/err.h"
#include "vmm_int.h"
#include "vmm_weak.h"

t_aspace	*vmm_sys_caller(t_process **out)
{
	t_process	*p;

	if (!proc_current)
		return (NULL);
	p = proc_current();
	if (!p || !p->aspace)
		return (NULL);
	*out = p;
	return (p->aspace);
}

int	vmm_sys_prot(uint64_t prot, uint32_t *fl)
{
	if (prot & ~(uint64_t)(PROT_R | PROT_W | PROT_X))
		return (E_INVAL);
	if (!(prot & PROT_R) || ((prot & PROT_W) && (prot & PROT_X)))
		return (E_INVAL);
	*fl = VM_R | VM_USER;
	if (prot & PROT_W)
		*fl |= VM_W;
	if (prot & PROT_X)
		*fl |= VM_X;
	return (0);
}

static int	user_len(uint64_t len, uint64_t *out)
{
	if (!len || len > USER_TOP - USER_MIN)
		return (E_INVAL);
	*out = align_up(len, PAGE_SIZE);
	return (0);
}

int64_t	vmm_sys_vfree(const t_sysargs *a)
{
	t_process	*p;
	t_aspace	*as;
	uint64_t	len;
	int			rc;

	as = vmm_sys_caller(&p);
	if (!as)
		return (E_PERM);
	if (user_len(a->a[1], &len) < 0)
		return (E_INVAL);
	rc = vmm_unmap(as, a->a[0], len);
	if (rc == 0 && secmaps_forget)
		secmaps_forget(p, a->a[0], len);
	return (rc);
}

int64_t	vmm_sys_vprotect(const t_sysargs *a)
{
	t_process	*p;
	t_aspace	*as;
	uint64_t	len;
	uint32_t	fl;

	as = vmm_sys_caller(&p);
	if (!as)
		return (E_PERM);
	if (user_len(a->a[1], &len) < 0 || vmm_sys_prot(a->a[2], &fl) < 0)
		return (E_INVAL);
	return (vmm_protect(as, a->a[0], len, fl & VM_PROT));
}
