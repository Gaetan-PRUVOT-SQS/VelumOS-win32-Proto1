#include "fake.h"

static void	tlb_drop(uint32_t cpu, uint64_t page)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_TLB)
	{
		if (g_fake.tlb_pa[cpu][i] && g_fake.tlb_va[cpu][i] == page)
			g_fake.tlb_pa[cpu][i] = 0;
		i++;
	}
}

static void	tlb_clear(uint32_t cpu)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_TLB)
		g_fake.tlb_pa[cpu][i++] = 0;
}

void	mmu_invlpg(uint64_t va)
{
	g_fake.invlpg++;
	g_fake.invlpg_cpu[g_fake.cpu]++;
	tlb_drop(g_fake.cpu, va & ~4095ull);
}

void	mmu_write_cr3(uint64_t pml4)
{
	g_fake.cr3 = pml4;
	tlb_clear(g_fake.cpu);
}

void	mmu_flush_global(void)
{
	g_fake.flush_cpu[g_fake.cpu]++;
	tlb_clear(g_fake.cpu);
}

void	fake_cpu_set(uint32_t cpu)
{
	g_fake.cpu = cpu;
}

void	fake_touch(uint32_t cpu, t_aspace *as, uintptr_t va)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_TLB && g_fake.tlb_pa[cpu][i])
		i++;
	if (i == FAKE_TLB)
		return ;
	g_fake.tlb_va[cpu][i] = va & ~4095ull;
	g_fake.tlb_pa[cpu][i] = fake_pte(as, va) & PTE_ADDR;
}

bool	fake_stale(uint32_t cpu, uintptr_t va)
{
	uint32_t	i;

	i = 0;
	while (i < FAKE_TLB)
	{
		if (g_fake.tlb_pa[cpu][i] && g_fake.tlb_va[cpu][i] == (va & ~4095ull))
			return (true);
		i++;
	}
	return (false);
}

void	fake_check_stale(uint64_t phys)
{
	uint32_t	c;
	uint32_t	i;

	c = 0;
	while (c < FAKE_CPUS)
	{
		i = 0;
		while (i < FAKE_TLB)
		{
			if (g_fake.tlb_pa[c][i] == phys)
				g_fake.stale_free++;
			i++;
		}
		c++;
	}
}

uint32_t	smp_cpu_count(void)
{
	return (g_fake.ncpu);
}

uint32_t	smp_cpu_id(void)
{
	return (g_fake.cpu);
}

int	smp_send_ipi(uint32_t cpu, uint8_t vec)
{
	uint32_t	self;

	g_fake.ipis++;
	if (g_fake.drop_ipi || vec != VMM_VEC_TLB || cpu >= g_fake.ncpu)
		return (0);
	self = g_fake.cpu;
	g_fake.cpu = cpu;
	vmm_tlb_ipi(NULL, NULL);
	g_fake.cpu = self;
	return (0);
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
	if (g_fake.relax_hook)
		g_fake.relax_hook();
}
