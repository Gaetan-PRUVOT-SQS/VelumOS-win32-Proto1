#include "velum/msr.h"
#include "vmm_int.h"

static uint64_t	pat_bits(uint64_t pat, uint64_t type)
{
	uint64_t	i;

	i = 1;
	while (i < 8)
	{
		if (((pat >> (i * 8)) & 0xff) == type)
			return (((i & 1) * PTE_PWT) | (((i >> 1) & 1) * PTE_PCD)
				| (((i >> 2) & 1) * PTE_PAT4K));
		i++;
	}
	return (0);
}

void	vmm_cpu_detect(void)
{
	uint64_t	pat;
	uint64_t	bits;

	g_vmm.nx = 0;
	if (msr_read(MSR_EFER) & EFER_NXE)
		g_vmm.nx = PTE_NX;
	g_vmm.uc_bits = PTE_PWT | PTE_PCD;
	g_vmm.wc_bits = g_vmm.uc_bits;
	if (!(mmu_cpuid_edx(1) & CPUID_PAT))
		return ;
	pat = msr_read(MSR_PAT);
	bits = pat_bits(pat, PAT_UC);
	if (bits)
		g_vmm.uc_bits = bits;
	bits = pat_bits(pat, PAT_WC);
	if (bits)
		g_vmm.wc_bits = bits;
	else
		g_vmm.wc_bits = g_vmm.uc_bits;
}
