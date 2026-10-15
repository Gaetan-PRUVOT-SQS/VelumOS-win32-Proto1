#include "fake.h"

void	mmu_invlpg(uint64_t va)
{
	(void)va;
	g_fake.invlpg++;
}

void	mmu_write_cr3(uint64_t pml4)
{
	g_fake.cr3 = pml4;
}

uint64_t	irq_save(void)
{
	return (0x200);
}

void	irq_restore(uint64_t flags)
{
	(void)flags;
}

void	cpu_relax(void)
{
}
