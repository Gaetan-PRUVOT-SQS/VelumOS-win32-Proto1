#ifndef BOOT_GUARD_H
# define BOOT_GUARD_H

# include <stdint.h>
# include "velum/err.h"

# define BG_PTE_PRESENT 1ULL
# define BG_PTE_HUGE (1ULL << 7)
# define BG_PTE_ADDR 0x000ffffffffff000ULL
# define BG_LEVELS 4

static inline uint64_t	*bg_table(uint64_t hhdm, uint64_t entry)
{
	return ((uint64_t *)(hhdm + (entry & BG_PTE_ADDR)));
}

static inline uint64_t	*bg_pte_find(uint64_t hhdm, uint64_t cr3, uint64_t va)
{
	uint64_t	*table;
	uint64_t	entry;
	int			level;

	table = bg_table(hhdm, cr3);
	level = BG_LEVELS - 1;
	while (level > 0)
	{
		entry = table[(va >> (12 + 9 * level)) & 511];
		if (!(entry & BG_PTE_PRESENT) || (entry & BG_PTE_HUGE))
			return ((uint64_t *)0);
		table = bg_table(hhdm, entry);
		level--;
	}
	return (&table[(va >> 12) & 511]);
}

static inline int	bg_unmap(uint64_t hhdm, uint64_t cr3, uint64_t va)
{
	uint64_t	*pte;

	if (va & 0xfffULL)
		return (E_INVAL);
	pte = bg_pte_find(hhdm, cr3, va);
	if (!pte || !(*pte & BG_PTE_PRESENT))
		return (E_NOTSUP);
	*pte &= ~BG_PTE_PRESENT;
	return (E_OK);
}

#endif
