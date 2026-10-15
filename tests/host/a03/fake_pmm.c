#include <string.h>
#include "fake.h"

uint64_t	pmm_alloc_zero(t_pmm_owner owner)
{
	uint32_t	n;
	uint32_t	i;

	if (g_fake.fail_after == 0)
		return (0);
	if (g_fake.fail_after > 0)
		g_fake.fail_after--;
	n = 0;
	while (n < FAKE_FRAMES)
	{
		i = (g_fake.cursor + n) % FAKE_FRAMES;
		if (!g_fake.used[i])
		{
			g_fake.used[i] = 1;
			g_fake.owner[i] = (uint8_t)owner;
			g_fake.live[owner]++;
			g_fake.cursor = i + 1;
			memset(g_fake.arena + (uint64_t)i * 4096, 0, 4096);
			return (FAKE_BASE + (uint64_t)i * 4096);
		}
		n++;
	}
	return (0);
}

void	pmm_free(uint64_t phys, t_pmm_owner owner)
{
	uint64_t	i;

	i = (phys - FAKE_BASE) / 4096;
	if (phys < FAKE_BASE || i >= FAKE_FRAMES || (phys & 4095)
		|| !g_fake.used[i] || g_fake.owner[i] != owner)
	{
		g_fake.errors++;
		return ;
	}
	g_fake.used[i] = 0;
	g_fake.live[owner]--;
	memset(g_fake.arena + i * 4096, 0xdd, 4096);
}

void	pmm_fail_after(int64_t n)
{
	g_fake.fail_after = n;
}

void	*phys_to_virt(uint64_t phys)
{
	if (phys < FAKE_BASE || phys >= FAKE_BASE + FAKE_FRAMES * 4096ull)
	{
		g_fake.errors++;
		return (g_fake.trash);
	}
	return (g_fake.arena + (phys - FAKE_BASE));
}
