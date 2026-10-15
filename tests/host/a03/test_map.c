#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	alloc_pages_zero_possedees(void)
{
	t_aspace	*as;
	uint64_t	pte;
	uint8_t		*p;

	fake_reset();
	as = fake_user();
	h_eq_i64("alloc 3 pages", vmm_alloc(as, UVA, 3 * 4096,
			VM_R | VM_W | VM_USER), 0);
	pte = fake_pte(as, UVA + 2 * 4096);
	h_true((pte & (PTE_P | PTE_W | PTE_U | PTE_OWNED | PTE_NX))
		== (PTE_P | PTE_W | PTE_U | PTE_OWNED | PTE_NX), "bits de la PTE");
	p = fake_ptr(as, UVA + 4096);
	h_true(p && p[0] == 0 && p[4095] == 0, "page a zero");
	h_eq_u64("frames user", g_fake.live[PMM_USER], 3);
	h_eq_u64("pages comptees", vmm_pages_used(as), 3 + 4 + 1);
	h_true(!fake_pte(as, UVA + 3 * 4096), "page suivante absente");
	vmm_aspace_destroy(as);
	h_eq_u64("frames rendues", g_fake.live[PMM_USER], 0);
	fake_clean("faux pmm propre");
}

static void	chevauchement_refuse(void)
{
	t_aspace	*as;
	t_vmreq		rq;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 2 * 4096, VM_R | VM_USER);
	h_eq_i64("chevauche", vmm_alloc(as, UVA + 4096, 2 * 4096,
			VM_R | VM_USER), E_EXIST);
	h_eq_i64("adjacent", vmm_alloc(as, UVA + 2 * 4096, 4096,
			VM_R | VM_USER), 0);
	rq.va = UVA - 4096;
	rq.pa = FAKE_BASE;
	rq.len = 2 * 4096;
	rq.flags = VM_R | VM_USER;
	h_eq_i64("map chevauche", vmm_map(as, &rq), E_EXIST);
	h_true(!fake_pte(as, UVA - 4096), "rien mappe avant");
	h_eq_u64("frames", g_fake.live[PMM_USER], 3);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	map_non_possede(void)
{
	t_aspace	*as;
	t_vmreq		rq;
	uint64_t	pte;

	fake_reset();
	as = fake_user();
	rq.pa = pmm_alloc_zero(PMM_DRIVER);
	rq.va = UVA;
	rq.len = 4096;
	rq.flags = VM_R | VM_W | VM_USER | VM_SHARED;
	h_eq_i64("map", vmm_map(as, &rq), 0);
	pte = fake_pte(as, UVA);
	h_true(!(pte & PTE_OWNED) && (pte & PTE_SHARED), "non possedee");
	h_eq_u64("adresse", pte & PTE_ADDR, rq.pa);
	h_eq_i64("unmap", vmm_unmap(as, UVA, 4096), 0);
	h_eq_u64("frame gardee", g_fake.live[PMM_DRIVER], 1);
	rq.pa += 1;
	h_eq_i64("pa non alignee", vmm_map(as, &rq), E_INVAL);
	rq.pa = 0xfffffffffffff000ull;
	h_eq_i64("pa deborde", vmm_map(as, &rq), E_INVAL);
	h_eq_i64("map nul", vmm_map(as, NULL), E_INVAL);
	vmm_aspace_destroy(as);
	h_eq_u64("frame toujours la", g_fake.live[PMM_DRIVER], 1);
	fake_clean("faux pmm propre");
}

static void	wx_refuse(void)
{
	t_aspace	*as;
	t_vmreq		rq;

	fake_reset();
	as = fake_user();
	h_eq_i64("alloc WX", vmm_alloc(as, UVA, 4096,
			VM_R | VM_W | VM_X | VM_USER), E_INVAL);
	rq.va = UVA;
	rq.pa = FAKE_BASE;
	rq.len = 4096;
	rq.flags = VM_R | VM_W | VM_X | VM_USER;
	h_eq_i64("map WX", vmm_map(as, &rq), E_INVAL);
	h_eq_i64("alloc RW", vmm_alloc(as, UVA, 4096, VM_R | VM_W | VM_USER), 0);
	h_eq_i64("protect WX", vmm_protect(as, UVA, 4096, VM_R | VM_W | VM_X),
		E_INVAL);
	h_true((fake_pte(as, UVA) & PTE_NX) != 0, "toujours NX");
	h_eq_i64("espace nul", vmm_alloc(NULL, UVA, 4096, VM_R), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/map");
	h_run("alloc_pages_zero_possedees", alloc_pages_zero_possedees);
	h_run("chevauchement_refuse", chevauchement_refuse);
	h_run("map_non_possede", map_non_possede);
	h_run("wx_refuse", wx_refuse);
	return (h_end());
}
