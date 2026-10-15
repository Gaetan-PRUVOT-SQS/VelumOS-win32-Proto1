#include <string.h>
#include "fake_cpuid.h"

static t_fakecpu	g_fake;

void	fake_reset(void)
{
	memset(&g_fake, 0, sizeof(g_fake));
}

void	fake_set(uint32_t leaf, const uint32_t regs[4])
{
	if (g_fake.n >= FAKE_LEAVES)
		return ;
	g_fake.leaf[g_fake.n] = leaf;
	memcpy(g_fake.regs[g_fake.n], regs, sizeof(g_fake.regs[0]));
	g_fake.n++;
}

void	fake_cpuid(uint32_t leaf, uint32_t sub, uint32_t out[4])
{
	int	i;

	(void)sub;
	if (g_fake.ncalls < FAKE_CALLS)
		g_fake.calls[g_fake.ncalls++] = leaf;
	i = 0;
	while (i < g_fake.n && g_fake.leaf[i] != leaf)
		i++;
	if (i < g_fake.n)
	{
		memcpy(out, g_fake.regs[i], sizeof(g_fake.regs[0]));
		return ;
	}
	out[0] = FAKE_GARBAGE;
	out[1] = FAKE_GARBAGE;
	out[2] = FAKE_GARBAGE;
	out[3] = FAKE_GARBAGE;
}

int	fake_called(uint32_t leaf)
{
	int	i;

	i = 0;
	while (i < g_fake.ncalls)
	{
		if (g_fake.calls[i] == leaf)
			return (1);
		i++;
	}
	return (0);
}

void	fake_brand(const char *brand)
{
	char		buf[48];
	uint32_t	regs[4];
	int			i;

	memset(buf, 0, sizeof(buf));
	memcpy(buf, brand, strnlen(brand, sizeof(buf)));
	i = 0;
	while (i < 3)
	{
		memcpy(regs, buf + 16 * i, 16);
		fake_set(0x80000002 + i, regs);
		i++;
	}
}
