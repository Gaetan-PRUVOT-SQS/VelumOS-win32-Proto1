#include "a02_fake.h"

int	g_corrupt_effective;

void	fake_corrupt_free(void)
{
	g_corrupt_effective = 1;
	g_pmm.stats.free_pages++;
}

void	fake_corrupt_owned(void)
{
	g_corrupt_effective = 1;
	g_pmm.stats.owned[PMM_KERNEL]++;
}

void	fake_corrupt_bit(void)
{
	g_corrupt_effective = 1;
	g_pmm.bits[g_pmm.hint / 64] |= 1ull << (g_pmm.hint % 64);
}
