#include "velum/boot.h"
#include "velum/err.h"
#include "cpu_int.h"

static uint64_t	*table_at(uint64_t entry)
{
	return ((uint64_t *)(boot_info()->hhdm + (entry & PTE_ADDR)));
}

static uint64_t	*pte_find(uint64_t va)
{
	uint64_t	*table;
	uint64_t	entry;
	int			level;

	table = table_at(cpu_read_cr3());
	level = 3;
	while (level > 0)
	{
		entry = table[(va >> (12 + 9 * level)) & 511];
		if (!(entry & PTE_PRESENT) || (entry & PTE_HUGE))
			return (NULL);
		table = table_at(entry);
		level--;
	}
	return (&table[(va >> 12) & 511]);
}

int	cpu_guard_page(uint64_t va)
{
	uint64_t	*pte;

	pte = pte_find(va);
	if (!pte || !(*pte & PTE_PRESENT))
		return (E_NOTSUP);
	*pte &= ~PTE_PRESENT;
	cpu_invlpg(va);
	return (E_OK);
}
