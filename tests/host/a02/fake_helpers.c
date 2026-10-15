#include "velum/err.h"
#include "a02_fake.h"

int	fake_simple(uint64_t first_mib, uint64_t last_mib)
{
	fake_reset();
	fake_range(first_mib * MIB, (last_mib - first_mib) * MIB, MEM_USABLE);
	return (fake_boot());
}

uint64_t	fake_drain(t_pmm_owner o, uint64_t *out, uint64_t cap)
{
	uint64_t	n;
	uint64_t	phys;

	n = 0;
	while (n < cap)
	{
		phys = pmm_alloc(o);
		if (!phys)
			break ;
		out[n] = phys;
		n++;
	}
	return (n);
}

void	fake_free_list(const uint64_t *list, uint64_t n, t_pmm_owner o)
{
	uint64_t	i;

	i = 0;
	while (i < n)
	{
		pmm_free(list[i], o);
		i++;
	}
}

uint64_t	fake_free_pages(void)
{
	t_pmm_stats	s;

	pmm_get_stats(&s);
	return (s.free_pages);
}

uint64_t	fake_owned(t_pmm_owner o)
{
	t_pmm_stats	s;

	pmm_get_stats(&s);
	return (s.owned[o]);
}
