#include "velum/err.h"
#include "vmm_int.h"

static uint64_t	audit_leaf(uint64_t e)
{
	return ((e & PTE_P) && (e & PTE_W) && !(e & PTE_NX));
}

static uint64_t	audit_pt(uint64_t phys)
{
	uint64_t	*t;
	uint64_t	bad;
	uint32_t	i;

	t = pt_table(phys);
	bad = 0;
	i = 0;
	while (i < PT_ENTRIES)
	{
		bad += audit_leaf(t[i]);
		i++;
	}
	return (bad);
}

static uint64_t	audit_pd(uint64_t phys)
{
	uint64_t	*t;
	uint64_t	bad;
	uint32_t	i;

	t = pt_table(phys);
	bad = 0;
	i = 0;
	while (i < PT_ENTRIES)
	{
		if ((t[i] & PTE_P) && (t[i] & PTE_PS))
			bad += audit_leaf(t[i]);
		else if (t[i] & PTE_P)
			bad += audit_pt(t[i]);
		i++;
	}
	return (bad);
}

static uint64_t	audit_pdpt(uint64_t phys)
{
	uint64_t	*t;
	uint64_t	bad;
	uint32_t	i;

	t = pt_table(phys);
	bad = 0;
	i = 0;
	while (i < PT_ENTRIES)
	{
		if ((t[i] & PTE_P) && (t[i] & PTE_PS))
			bad += audit_leaf(t[i]);
		else if (t[i] & PTE_P)
			bad += audit_pd(t[i]);
		i++;
	}
	return (bad);
}

int	vmm_audit_wx(void)
{
	uint64_t	*pml4;
	uint64_t	irq;
	uint64_t	bad;
	uint32_t	i;

	if (!g_vmm.nx || !g_vmm.kas.pml4)
		return (E_NOTSUP);
	irq = vmm_lock(&g_vmm.kas);
	pml4 = pt_table(g_vmm.kas.pml4);
	bad = 0;
	i = PT_ENTRIES / 2;
	while (i < PT_ENTRIES)
	{
		if (pml4[i] & PTE_P)
			bad += audit_pdpt(pml4[i]);
		i++;
	}
	vmm_unlock(&g_vmm.kas, irq);
	return ((int)min_u64(bad, 0x7fffffff));
}
