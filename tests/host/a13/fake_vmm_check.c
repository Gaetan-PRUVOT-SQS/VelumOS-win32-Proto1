#include "fakes.h"

bool	fake_vmm_guards_ok(void)
{
	int	i;

	i = 0;
	while (i < FAKE_WIN)
	{
		if (g_fvmm.win[i].live)
		{
			if (g_fvmm.win[i].raw[0] != 0xa5)
				return (false);
			if (g_fvmm.win[i].p[g_fvmm.win[i].len] != 0xa5)
				return (false);
		}
		i++;
	}
	return (!g_fvmm.guard_broken);
}

int	fake_vmm_live_windows(void)
{
	int	i;
	int	n;

	i = 0;
	n = 0;
	while (i < FAKE_WIN)
	{
		n += g_fvmm.win[i].live;
		i++;
	}
	return (n);
}

int	fake_vmm_live_maps(void)
{
	int	i;
	int	n;

	i = 0;
	n = 0;
	while (i < FAKE_MAP)
	{
		n += g_fvmm.map[i].live;
		i++;
	}
	return (n);
}

const t_fmap	*fx_live_map(int n)
{
	int	i;

	i = 0;
	while (i < FAKE_MAP)
	{
		if (g_fvmm.map[i].live && n-- == 0)
			return (&g_fvmm.map[i]);
		i++;
	}
	return (NULL);
}
