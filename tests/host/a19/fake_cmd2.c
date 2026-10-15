#include "fake.h"

static int64_t	g_now;

int	fake_cmd_find(uint32_t id, uint32_t code)
{
	int	i;
	int	n;

	i = 0;
	n = 0;
	while (i < g_fake_cmds.n)
	{
		if (g_fake_cmds.list[i].id == id && g_fake_cmds.list[i].code == code)
			n++;
		i++;
	}
	return (n);
}

int	fake_cmd_cbs(void)
{
	return (g_fake_cmds.cbs);
}

void	fake_clock_set(int64_t ns)
{
	g_now = ns;
}

int64_t	fake_clock_now(void)
{
	return (g_now);
}
