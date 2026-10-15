#include "a02_fake.h"

uint64_t	fake_drain_to(t_pmm_owner o, uint64_t *out, uint64_t cap,
				uint64_t max)
{
	uint64_t	n;
	uint64_t	phys;

	n = 0;
	while (n < cap)
	{
		phys = pmm_alloc_pages(o, 1, 1, max);
		if (!phys)
			break ;
		out[n] = phys;
		n++;
	}
	return (n);
}

uint64_t	fake_mixed_alloc(uint64_t i)
{
	if (i % 3 == 0)
		return (pmm_alloc(PMM_KERNEL));
	if (i % 3 == 1)
		return (pmm_alloc_zero(PMM_KERNEL));
	return (pmm_alloc_pages(PMM_KERNEL, 1, 1, 0));
}

uint64_t	fake_burst(uint64_t *list)
{
	uint64_t	n;
	uint64_t	i;
	uint64_t	phys;

	n = 0;
	i = 0;
	while (i < 10)
	{
		phys = fake_mixed_alloc(i);
		if (phys)
			list[n++] = phys;
		i++;
	}
	return (n);
}

int	fake_region_machine(void)
{
	fake_reset();
	fake_range(1 * MIB, 16 * MIB, MEM_USABLE);
	fake_range(17 * MIB, 2 * MIB, MEM_BOOT_RECLAIM);
	fake_range(19 * MIB, MIB, MEM_RESERVED);
	fake_range(20 * MIB, 2 * MIB, MEM_BOOT_RECLAIM);
	return (fake_boot());
}

uint64_t	fake_churn(uint64_t *live, uint64_t *nlive, uint64_t target,
				uint64_t ops)
{
	uint64_t	allocs;
	uint64_t	pick;
	uint64_t	phys;

	allocs = 0;
	while (ops--)
	{
		if (*nlive < target)
		{
			phys = pmm_alloc(PMM_KERNEL);
			allocs++;
			if (phys)
				live[(*nlive)++] = phys;
			continue ;
		}
		pick = fake_below(*nlive);
		pmm_free(live[pick], PMM_KERNEL);
		live[pick] = live[--(*nlive)];
	}
	return (allocs);
}
