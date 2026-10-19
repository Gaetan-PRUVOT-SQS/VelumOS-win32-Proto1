#include "velum/libk.h"
#include "smp_int.h"

static void	put64(uint8_t *page, uint32_t off, uint64_t value)
{
	memcpy(page + off, &value, sizeof(value));
}

static void	put_far(uint8_t *page, uint32_t off, uint32_t target, uint16_t sel)
{
	memcpy(page + off, &target, sizeof(target));
	memcpy(page + off + sizeof(target), &sel, sizeof(sel));
}

void	smp_tramp_data(uint8_t *page, const t_tramp_cfg *cfg)
{
	uint16_t	limit;
	uint32_t	gdt_base;

	limit = TRAMP_GDT_ENTRIES * 8 - 1;
	gdt_base = (uint32_t)(cfg->base + TD_GDT);
	memcpy(page + TD_GDTR, &limit, sizeof(limit));
	memcpy(page + TD_GDTR + 2, &gdt_base, sizeof(gdt_base));
	put_far(page, TD_FAR32, (uint32_t)cfg->base + cfg->off_pm32,
		TRAMP_SEL_CODE32);
	put_far(page, TD_FAR64, (uint32_t)cfg->base + cfg->off_lm64,
		TRAMP_SEL_CODE64);
	put64(page, TD_CR3, cfg->pml4_phys);
	put64(page, TD_ENTRY, cfg->entry);
	put64(page, TD_GDT, 0);
	put64(page, TD_GDT + 8, TRAMP_GDT_CODE64);
	put64(page, TD_GDT + 16, TRAMP_GDT_DATA);
	put64(page, TD_GDT + 24, TRAMP_GDT_CODE32);
}

void	smp_tramp_target(uint8_t *page, uint64_t stack_top, uint64_t arg)
{
	put64(page, TD_STACK, stack_top);
	put64(page, TD_ARG, arg);
}

void	smp_tramp_tables(uint64_t *pml4, uint64_t *pdpt, uint64_t *pd,
	const uint64_t *kernel_pml4, const t_tramp_cfg *cfg)
{
	uint32_t	i;

	memset(pml4, 0, TRAMP_PML4_ENTRIES * sizeof(uint64_t));
	memset(pdpt, 0, TRAMP_PML4_ENTRIES * sizeof(uint64_t));
	memset(pd, 0, TRAMP_PML4_ENTRIES * sizeof(uint64_t));
	pml4[0] = cfg->pdpt_phys | TRAMP_PTE_PRESENT_RW;
	pdpt[0] = cfg->pd_phys | TRAMP_PTE_PRESENT_RW;
	pd[0] = TRAMP_PDE_HUGE_RW;
	i = TRAMP_KERNEL_FIRST;
	while (i < TRAMP_PML4_ENTRIES)
	{
		pml4[i] = kernel_pml4[i];
		i++;
	}
}
