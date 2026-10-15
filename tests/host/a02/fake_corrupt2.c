#include "a02_fake.h"

void	fake_corrupt_disarm(void)
{
	if (!g_pmm.fail_armed)
		return ;
	g_corrupt_effective = 1;
	g_pmm.fail_armed = 0;
}

void	fake_corrupt_owner(void)
{
	g_corrupt_effective = 1;
	g_pmm.owner[g_pmm.hint - 1] = PMM_HEAP;
}

int	fake_corrupt_took_effect(void)
{
	return (g_corrupt_effective);
}

void	fake_corrupt_reset(void)
{
	g_corrupt_effective = 0;
}
