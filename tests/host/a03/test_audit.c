#include "velum/err.h"
#include "harness.h"
#include "fake.h"

static void	audit_propre(void)
{
	t_vmreq	rq;

	fake_reset();
	vmm_kstack_alloc(4);
	vmm_io_map(0xfee00000, 4096, VM_R | VM_W);
	g_vmm.ready = false;
	rq.va = KERNEL_BASE;
	rq.pa = 0x200000;
	rq.len = 0x400000;
	vmm_map_span(&g_vmm.kas, &rq, PTE_P | PTE_G);
	rq.va = HHDM_DEFAULT;
	rq.pa = 0;
	vmm_map_span(&g_vmm.kas, &rq, PTE_P | PTE_W | PTE_NX);
	g_vmm.ready = true;
	h_eq_i64("aucune page W et X", vmm_audit_wx(), 0);
}

static void	audit_detecte_wx(void)
{
	uint64_t	*e;

	fake_reset();
	e = vmm_entry(&g_vmm.kas, KHEAP_BASE, PT_LEAF, true);
	*e = FAKE_BASE | PTE_P | PTE_W;
	h_eq_i64("une page 4 Kio", vmm_audit_wx(), 1);
	e = vmm_entry(&g_vmm.kas, KIO_BASE, PT_PD, true);
	*e = vmm_pte_large(PTE_P | PTE_W);
	h_eq_i64("plus une page 2 Mio", vmm_audit_wx(), 2);
	g_vmm.nx = 0;
	h_eq_i64("sans NX", vmm_audit_wx(), E_NOTSUP);
	g_vmm.nx = PTE_NX;
	fake_clean("faux pmm propre");
}

static void	invariants_noyau(void)
{
	t_vmreq	rq;

	fake_reset();
	h_true(!vmm_entry(&g_vmm.kas, HHDM_DEFAULT + 5 * PML4_SPAN, PT_PDPT,
			true), "pas de nouvelle entree PML4 haute apres le demarrage");
	g_vmm.ready = false;
	rq.va = HHDM_DEFAULT;
	rq.pa = 0;
	rq.len = 0x400000;
	pmm_fail_after(0);
	h_eq_i64("tables de demarrage sans memoire", vmm_map_span(&g_vmm.kas, &rq,
			PTE_P), E_NOMEM);
	pmm_fail_after(-1);
	vmm_switch(NULL);
	h_eq_u64("bascule ignoree avant le demarrage", g_fake.cr3, 0);
	g_vmm.ready = true;
	h_eq_u64("pages d'un espace nul", vmm_pages_used(NULL), 0);
	vmm_aspace_destroy(&g_vmm.kas);
	h_eq_i64("destruction du noyau refusee", g_fake.errors, 1);
	h_true(g_vmm.kas.pml4 != 0, "noyau intact");
}

int	main(void)
{
	h_begin("a03/audit");
	h_run("audit_propre", audit_propre);
	h_run("audit_detecte_wx", audit_detecte_wx);
	h_run("invariants_noyau", invariants_noyau);
	return (h_end());
}
