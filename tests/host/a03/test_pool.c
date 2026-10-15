#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	echec_creation_et_pool(void)
{
	t_aspace	*as;
	uintptr_t	va;

	fake_reset();
	pmm_fail_after(0);
	h_true(vmm_aspace_create() == NULL, "page d'espace refusee");
	pmm_fail_after(1);
	h_true(vmm_aspace_create() == NULL, "pml4 refusee");
	pmm_fail_after(-1);
	h_eq_u64("rien garde", g_fake.live[PMM_KERNEL], 0);
	as = fake_user();
	va = UVA;
	while (as->nfree > 0 && vmm_alloc(as, va, 4096, VM_R | VM_USER) == 0)
		va += 2 * 4096;
	pmm_fail_after(0);
	h_eq_i64("pool epuise alloc", vmm_alloc(as, va, 4096, VM_R | VM_USER),
		E_NOMEM);
	h_eq_i64("pool epuise unmap", vmm_unmap(as, UVA, 4096), E_NOMEM);
	pmm_fail_after(-1);
	h_true(fake_pte(as, UVA) != 0, "rien demappe");
	h_true(!fake_pte(as, va), "rien mappe");
	vmm_aspace_destroy(as);
	h_eq_u64("tout rendu", g_fake.live[PMM_KERNEL] + g_fake.live[PMM_USER], 0);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/pool");
	h_run("echec_creation_et_pool", echec_creation_et_pool);
	return (h_end());
}
