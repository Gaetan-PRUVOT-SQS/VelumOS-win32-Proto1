#include "vmm_int.h"

static uint64_t	cache_bits(uint32_t fl)
{
	if (fl & VM_NOCACHE)
		return (g_vmm.uc_bits);
	if (fl & VM_WC)
		return (g_vmm.wc_bits);
	return (0);
}

uint64_t	vmm_pte_bits(uint32_t fl)
{
	uint64_t	bits;

	bits = PTE_P | cache_bits(fl);
	if (fl & VM_W)
		bits |= PTE_W;
	if (fl & VM_USER)
		bits |= PTE_U;
	if (!(fl & VM_X))
		bits |= g_vmm.nx;
	if (fl & VM_GLOBAL)
		bits |= PTE_G;
	if (fl & VM_SHARED)
		bits |= PTE_SHARED;
	return (bits);
}

uint64_t	vmm_pte_large(uint64_t bits)
{
	if (bits & PTE_PAT4K)
		bits = (bits & ~PTE_PAT4K) | PTE_PAT2M;
	return (bits | PTE_PS);
}

static uint32_t	cache_flags(uint64_t pte, uint64_t size)
{
	uint64_t	cache;

	cache = pte & (PTE_PWT | PTE_PCD);
	if (size == PAGE_SIZE && (pte & PTE_PAT4K))
		cache |= PTE_PAT4K;
	if (size > PAGE_SIZE && (pte & PTE_PAT2M))
		cache |= PTE_PAT4K;
	if (!cache)
		return (0);
	if (cache == g_vmm.uc_bits)
		return (VM_NOCACHE);
	if (cache == g_vmm.wc_bits)
		return (VM_WC);
	return (0);
}

uint32_t	vmm_pte_flags(uint64_t pte, uint64_t size)
{
	uint32_t	fl;

	fl = VM_R | cache_flags(pte, size);
	if (pte & PTE_W)
		fl |= VM_W;
	if (!(pte & PTE_NX))
		fl |= VM_X;
	if (pte & PTE_U)
		fl |= VM_USER;
	if (pte & PTE_G)
		fl |= VM_GLOBAL;
	if (pte & PTE_SHARED)
		fl |= VM_SHARED;
	return (fl);
}
