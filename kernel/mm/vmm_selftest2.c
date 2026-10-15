#include "velum/err.h"
#include "velum/klog.h"
#include "vmm_int.h"
#include "vmm_weak.h"

#define ST_UVA 0x400000ull

static int	st_kernel_copy(void)
{
	uint64_t	v;
	char		s[8];
	int			fails;

	v = 0;
	fails = copy_from_user(&v, (t_uptr)(uintptr_t)s, sizeof(v)) != E_FAULT;
	fails += copy_from_user(&v, 0, sizeof(v)) != E_FAULT;
	fails += copy_to_user(KERNEL_BASE, &v, sizeof(v)) != E_FAULT;
	fails += strncpy_from_user(s, HHDM_DEFAULT, sizeof(s)) != E_FAULT;
	fails += !user_range_ok(0, 0);
	if (fails)
		klog_err("vmm selftest: copie depuis une adresse noyau acceptee");
	return (fails != 0);
}

static int	st_copy(void)
{
	uint64_t	v;
	uint64_t	w;
	int			fails;

	v = 0x1122334455667788ull;
	w = 0;
	fails = copy_to_user(ST_UVA + PAGE_SIZE - 4, &v, 8) != 0;
	fails += copy_from_user(&w, ST_UVA + PAGE_SIZE - 4, 8) != 0;
	fails += w != v;
	fails += copy_to_user(ST_UVA + 2 * PAGE_SIZE, &v, 8) != E_FAULT;
	fails += copy_from_user(&w, ST_UVA + 2 * PAGE_SIZE, 8) != 0;
	fails += copy_from_user(&w, ST_UVA + 3 * PAGE_SIZE - 4, 8) != E_FAULT;
	fails += copy_from_user(&w, KERNEL_BASE, 8) != E_FAULT;
	return (fails);
}

static int	st_aspace(t_aspace *as)
{
	int	fails;

	fails = vmm_alloc(as, ST_UVA, 2 * PAGE_SIZE, VM_R | VM_W | VM_USER) != 0;
	fails += vmm_alloc(as, ST_UVA + 2 * PAGE_SIZE, PAGE_SIZE,
			VM_R | VM_USER) != 0;
	fails += vmm_alloc(as, ST_UVA, PAGE_SIZE, VM_R | VM_USER) != E_EXIST;
	fails += vmm_alloc(as, ST_UVA + 3 * PAGE_SIZE, PAGE_SIZE,
			VM_R | VM_W | VM_X | VM_USER) != E_INVAL;
	vmm_switch(as);
	if (!fails)
		fails += st_copy();
	vmm_switch(NULL);
	return (fails);
}

int	vmm_st_user(void)
{
	t_pmm_stats	before;
	t_pmm_stats	after;
	t_aspace	*as;
	int			fails;

	fails = st_kernel_copy();
	pmm_get_stats(&before);
	as = vmm_aspace_create();
	if (!as)
	{
		klog_err("vmm selftest: creation d'espace impossible");
		return (fails + 1);
	}
	fails += st_aspace(as);
	vmm_aspace_destroy(as);
	pmm_get_stats(&after);
	if (before.free_pages != after.free_pages)
		fails++;
	if (fails)
		klog_err("vmm selftest: espace utilisateur (%d)", fails);
	return (fails != 0);
}
