#include "fake.h"

uint64_t	fake_pte(t_aspace *as, uintptr_t va)
{
	t_ptlook	look;

	if (!vmm_lookup(as, va, &look))
		return (0);
	return (look.pte);
}

uint8_t	*fake_ptr(t_aspace *as, uintptr_t va)
{
	t_ptlook	look;

	if (!vmm_lookup(as, va, &look))
		return (NULL);
	return (phys_to_virt(look.pa));
}

t_aspace	*fake_user_env(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 2 * 4096, VM_R | VM_W | VM_USER);
	vmm_alloc(as, UVA + 2 * 4096, 4096, VM_R | VM_USER);
	vmm_alloc(as, UVA + 4 * 4096, 4096, VM_R | VM_W);
	return (as);
}
