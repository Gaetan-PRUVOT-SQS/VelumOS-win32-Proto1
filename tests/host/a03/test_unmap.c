#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	unmap_partiel_decoupe(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4 * 4096, VM_R | VM_W | VM_USER);
	h_eq_i64("unmap milieu", vmm_unmap(as, UVA + 4096, 4096), 0);
	h_eq_u64("frames", g_fake.live[PMM_USER], 3);
	h_true(!fake_pte(as, UVA + 4096), "page retiree");
	h_true(fake_pte(as, UVA) && fake_pte(as, UVA + 2 * 4096), "voisines");
	h_eq_u64("trou retrouve", vmm_find_free(as, 4096, UVA, USER_TOP),
		UVA + 4096);
	h_eq_u64("deux regions", (uint64_t)(as->regions && as->regions->next
			&& !as->regions->next->next), 1);
	h_eq_u64("region basse", as->regions->end, UVA + 4096);
	h_eq_u64("region haute", as->regions->next->start, UVA + 2 * 4096);
	h_eq_i64("unmap non aligne", vmm_unmap(as, UVA + 1, 4096), E_INVAL);
	h_eq_i64("unmap nul", vmm_unmap(NULL, UVA, 4096), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	unmap_trous_et_elagage(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4096, VM_R | VM_USER);
	vmm_alloc(as, 0x7f0000000000ull, 2 * 4096, VM_R | VM_W | VM_USER);
	h_eq_u64("tables creees", as->tables, 7);
	h_eq_i64("vfree de tout l'espace", vmm_unmap(as, USER_MIN,
			USER_TOP - USER_MIN), 0);
	h_eq_u64("frames rendues", g_fake.live[PMM_USER], 0);
	h_eq_u64("tables elaguees", as->tables, 1);
	h_eq_u64("tables pmm", g_fake.live[PMM_PAGETABLE], KH_BASELINE + 1);
	h_true(as->regions == NULL, "plus de region");
	h_eq_u64("pages comptees", vmm_pages_used(as), 2);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	invlpg_espace_actif(void)
{
	t_aspace	*as;
	uint64_t	before;

	fake_reset();
	as = fake_user();
	h_eq_u64("cr3 charge", g_fake.cr3, as->pml4);
	vmm_alloc(as, UVA, 4 * 4096, VM_R | VM_USER);
	before = g_fake.invlpg;
	vmm_unmap(as, UVA, 2 * 4096);
	h_true(g_fake.invlpg >= before + 2, "invlpg sur l'espace actif");
	vmm_switch(NULL);
	h_eq_u64("cr3 noyau", g_fake.cr3, g_vmm.kas.pml4);
	before = g_fake.invlpg;
	vmm_unmap(as, UVA + 2 * 4096, 2 * 4096);
	h_eq_u64("pas d'invlpg inactif", g_fake.invlpg, before);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	destruction_sans_fuite(void)
{
	t_aspace	*as;
	t_vmreq		rq;
	uintptr_t	va;

	fake_reset();
	as = fake_user();
	rq.pa = pmm_alloc_zero(PMM_FB);
	rq.va = 0x10000000;
	rq.len = 4096;
	rq.flags = VM_R | VM_W | VM_WC | VM_USER;
	h_eq_i64("map fb", vmm_map(as, &rq), 0);
	va = USER_MIN;
	while (va < 0x7f0000000000ull)
	{
		vmm_alloc(as, va, 3 * 4096, VM_R | VM_USER);
		va = va * 4 + 0x40000000;
	}
	vmm_aspace_destroy(as);
	h_eq_u64("user", g_fake.live[PMM_USER], 0);
	h_eq_u64("tables", g_fake.live[PMM_PAGETABLE], KH_BASELINE);
	h_eq_u64("pool", g_fake.live[PMM_KERNEL], 0);
	h_eq_u64("fb garde", g_fake.live[PMM_FB], 1);
	h_true(g_vmm.current == &g_vmm.kas, "bascule vers le noyau");
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/unmap");
	h_run("unmap_partiel_decoupe", unmap_partiel_decoupe);
	h_run("unmap_trous_et_elagage", unmap_trous_et_elagage);
	h_run("invlpg_espace_actif", invlpg_espace_actif);
	h_run("destruction_sans_fuite", destruction_sans_fuite);
	return (h_end());
}
