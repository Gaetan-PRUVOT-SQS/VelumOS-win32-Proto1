#include "harness.h"
#include "fake.h"

static void	indices_9bits(void)
{
	h_eq_u64("image pml4", pt_index(KERNEL_BASE, PT_PML4), 511);
	h_eq_u64("image pdpt", pt_index(KERNEL_BASE, PT_PDPT), 510);
	h_eq_u64("image pd", pt_index(KERNEL_BASE, PT_PD), 0);
	h_eq_u64("image pt", pt_index(KERNEL_BASE + 0x5000, PT_LEAF), 5);
	h_eq_u64("haut user pml4", pt_index(USER_TOP - 4096, PT_PML4), 255);
	h_eq_u64("haut user pt", pt_index(USER_TOP - 4096, PT_LEAF), 510);
	h_eq_u64("hhdm pml4", pt_index(HHDM_DEFAULT, PT_PML4), 256);
	h_eq_u64("tas pml4", pt_index(KHEAP_BASE, PT_PML4), 384);
	h_eq_u64("pile pml4", pt_index(KSTACK_BASE, PT_PML4), 448);
	h_eq_u64("portee pdpt", pt_span(PT_PDPT), 0x40000000ull);
}

static void	drapeaux_vers_pte(void)
{
	fake_reset();
	h_eq_u64("R", vmm_pte_bits(VM_R), PTE_P | PTE_NX);
	h_eq_u64("RW", vmm_pte_bits(VM_R | VM_W), PTE_P | PTE_W | PTE_NX);
	h_eq_u64("RX", vmm_pte_bits(VM_R | VM_X), PTE_P);
	h_eq_u64("user", vmm_pte_bits(VM_R | VM_USER), PTE_P | PTE_U | PTE_NX);
	h_eq_u64("global", vmm_pte_bits(VM_R | VM_GLOBAL),
		PTE_P | PTE_G | PTE_NX);
	h_eq_u64("UC", vmm_pte_bits(VM_R | VM_NOCACHE),
		PTE_P | PTE_NX | PTE_PCD | PTE_PWT);
	h_eq_u64("WC", vmm_pte_bits(VM_R | VM_WC), PTE_P | PTE_NX | PTE_PAT4K);
	h_eq_u64("partage", vmm_pte_bits(VM_R | VM_SHARED),
		PTE_P | PTE_NX | PTE_SHARED);
	g_vmm.nx = 0;
	h_eq_u64("sans NX", vmm_pte_bits(VM_R | VM_W), PTE_P | PTE_W);
	g_vmm.nx = PTE_NX;
}

static void	pte_vers_drapeaux(void)
{
	uint64_t	wc2m;
	uint32_t	fl;

	fake_reset();
	fl = VM_R | VM_W | VM_USER;
	h_eq_u64("aller retour RWU", vmm_pte_flags(vmm_pte_bits(fl), 4096), fl);
	fl = VM_R | VM_X | VM_GLOBAL;
	h_eq_u64("aller retour RXG", vmm_pte_flags(vmm_pte_bits(fl), 4096), fl);
	fl = VM_R | VM_NOCACHE | VM_W;
	h_eq_u64("aller retour UC", vmm_pte_flags(vmm_pte_bits(fl), 4096), fl);
	fl = VM_R | VM_WC | VM_SHARED;
	h_eq_u64("aller retour WC", vmm_pte_flags(vmm_pte_bits(fl), 4096), fl);
	wc2m = vmm_pte_large(vmm_pte_bits(VM_R | VM_WC));
	h_true((wc2m & PTE_PAT2M) && (wc2m & PTE_PS), "grande page PAT bit 12");
	h_eq_u64("grande page WC", vmm_pte_flags(wc2m, PAGE_2M), VM_R | VM_WC);
	h_eq_u64("grande page WB", vmm_pte_flags(vmm_pte_large(PTE_P), PAGE_2M),
		VM_R | VM_X);
}

static void	creation_tables(void)
{
	t_aspace	*as;
	uint64_t	*e;
	uint64_t	*pml4;
	t_ptlook	look;

	fake_reset();
	as = fake_user();
	h_eq_u64("trou pml4 vide", vmm_reach(as, UVA, PT_LEAF, &e), PML4_SPAN);
	h_true(e == NULL, "pas d'entree sans tables");
	e = vmm_entry(as, UVA + 0x123000, PT_LEAF, true);
	h_eq_u64("3 tables creees", as->tables, 4);
	pml4 = pt_table(as->pml4);
	h_eq_u64("intermediaire P W U", pml4[0] & 7, 7);
	*e = (FAKE_BASE + 0x5000) | vmm_pte_bits(VM_R | VM_USER);
	h_true(vmm_lookup(as, UVA + 0x123abc, &look), "lookup");
	h_eq_u64("adresse avec decalage", look.pa, FAKE_BASE + 0x5abc);
	h_true(!(look.pte & PTE_W), "W effectif absent");
	h_true(pml4[256] == pt_table(g_vmm.kas.pml4)[256], "moitie haute");
	h_true(!vmm_lookup(as, 0x0000800000000000ull, &look), "non canonique");
	*e = 0;
	vmm_aspace_destroy(as);
	h_eq_u64("tout rendu", g_fake.live[PMM_PAGETABLE], KH_BASELINE);
	fake_clean("faux pmm propre");
}

int	main(void)
{
	h_begin("a03/walk");
	h_run("indices_9bits", indices_9bits);
	h_run("drapeaux_vers_pte", drapeaux_vers_pte);
	h_run("pte_vers_drapeaux", pte_vers_drapeaux);
	h_run("creation_tables", creation_tables);
	return (h_end());
}
