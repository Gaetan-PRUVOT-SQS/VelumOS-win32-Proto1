#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	protect_change_droits(void)
{
	t_aspace	*as;
	uint64_t	before;
	uint64_t	pte;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 3 * 4096, VM_R | VM_W | VM_USER);
	before = fake_pte(as, UVA + 4096) & PTE_ADDR;
	h_eq_i64("R", vmm_protect(as, UVA + 4096, 4096, VM_R), 0);
	pte = fake_pte(as, UVA + 4096);
	h_true(!(pte & PTE_W) && (pte & PTE_NX) && (pte & PTE_OWNED), "R");
	h_eq_u64("meme frame", pte & PTE_ADDR, before);
	h_eq_i64("RX", vmm_protect(as, UVA + 4096, 4096, VM_R | VM_X), 0);
	h_true(!(fake_pte(as, UVA + 4096) & PTE_NX), "X");
	h_true((fake_pte(as, UVA) & PTE_W) != 0, "voisine intacte");
	h_eq_u64("region milieu", vmm_reg_from(as, UVA + 4096)->flags,
		VM_R | VM_X | VM_USER);
	h_eq_u64("region basse", vmm_reg_from(as, UVA)->end, UVA + 4096);
	h_eq_i64("bits hors R/W/X", vmm_protect(as, UVA, 4096, VM_R | VM_USER),
		E_INVAL);
	h_eq_i64("sans R", vmm_protect(as, UVA, 4096, VM_W), E_INVAL);
	h_eq_i64("nul", vmm_protect(NULL, UVA, 4096, VM_R), E_INVAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	protect_trou_refuse(void)
{
	t_aspace	*as;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4096, VM_R | VM_W | VM_USER);
	vmm_alloc(as, UVA + 2 * 4096, 4096, VM_R | VM_W | VM_USER);
	h_eq_i64("trou", vmm_protect(as, UVA, 3 * 4096, VM_R), E_NOENT);
	h_true((fake_pte(as, UVA) & PTE_W) != 0, "rien change");
	h_eq_i64("sans tables", vmm_protect(as, 0x7f0000000000ull, 4096, VM_R),
		E_NOENT);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

static void	protect_plafond_section(void)
{
	t_aspace	*as;
	t_vmreq		rq;

	fake_reset();
	as = fake_user();
	rq.pa = pmm_alloc_zero(PMM_USER);
	rq.va = UVA;
	rq.len = 4096;
	rq.flags = VM_R | VM_USER | VM_SHARED;
	vmm_map(as, &rq);
	h_eq_i64("section RO vers RW", vmm_protect(as, UVA, 4096, VM_R | VM_W),
		E_ACCES);
	h_eq_i64("section RO vers RX", vmm_protect(as, UVA, 4096, VM_R | VM_X),
		E_ACCES);
	h_eq_i64("section RO vers R", vmm_protect(as, UVA, 4096, VM_R), 0);
	vmm_alloc(as, UVA + 4096, 4096, VM_R | VM_USER);
	h_eq_i64("anonyme vers RW", vmm_protect(as, UVA + 4096, 4096,
			VM_R | VM_W), 0);
	h_eq_i64("melange vers RW", vmm_protect(as, UVA, 2 * 4096,
			VM_R | VM_W), E_ACCES);
	vmm_aspace_destroy(as);
	pmm_free(rq.pa, PMM_USER);
	fake_clean("faux pmm propre");
}

static void	query_drapeaux(void)
{
	t_aspace	*as;
	t_vminfo	info;
	t_vmreq		rq;

	fake_reset();
	as = fake_user();
	vmm_alloc(as, UVA, 4096, VM_R | VM_W | VM_USER);
	h_true(vmm_query(as, UVA + 0x10, &info), "query");
	h_eq_u64("flags", info.flags, VM_R | VM_W | VM_USER);
	h_eq_u64("decalage", info.pa & 4095, 0x10);
	h_true(!vmm_query(as, UVA + 4096, &info), "absent");
	h_true(!vmm_query(as, 0x0000800000001000ull, &info), "non canonique");
	h_true(!vmm_query(NULL, UVA, &info), "nul");
	g_vmm.ready = false;
	rq.va = HHDM_DEFAULT;
	rq.pa = 0;
	rq.len = PAGE_2M;
	vmm_map_span(&g_vmm.kas, &rq, vmm_pte_bits(VM_R | VM_WC | VM_GLOBAL));
	g_vmm.ready = true;
	h_true(vmm_query(&g_vmm.kas, HHDM_DEFAULT + 0x12345, &info), "2 Mio");
	h_eq_u64("pa 2 Mio", info.pa, 0x12345);
	h_eq_u64("WC 2 Mio", info.flags, VM_R | VM_WC | VM_GLOBAL);
	vmm_aspace_destroy(as);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/protect");
	h_run("protect_change_droits", protect_change_droits);
	h_run("protect_trou_refuse", protect_trou_refuse);
	h_run("protect_plafond_section", protect_plafond_section);
	h_run("query_drapeaux", query_drapeaux);
	return (h_end());
}
