#include <stdlib.h>
#include "fakes.h"
#include "velum/vmm.h"

t_fvmm	g_fvmm;

void	fake_vmm_reset(void)
{
	int	i;

	i = 0;
	while (i < FAKE_WIN)
	{
		if (g_fvmm.win[i].live)
			free(g_fvmm.win[i].raw);
		i++;
	}
	memset(&g_fvmm, 0, sizeof(g_fvmm));
}

static t_fwin	*slot(void)
{
	int	i;

	i = 0;
	while (i < FAKE_WIN)
	{
		if (!g_fvmm.win[i].live)
			return (&g_fvmm.win[i]);
		i++;
	}
	return (NULL);
}

void	*vmm_io_map(uint64_t phys, size_t len, uint32_t flags)
{
	t_fwin		*w;
	uint32_t	call;

	call = g_fvmm.io_calls++;
	g_fvmm.last_io_flags = flags;
	if (call < 32 && ((g_fvmm.fail_io_mask >> call) & 1))
		return (NULL);
	w = slot();
	if (!w)
		return (NULL);
	w->raw = malloc(len + 2 * FAKE_GUARD);
	if (!w->raw)
		return (NULL);
	memset(w->raw, 0xa5, len + 2 * FAKE_GUARD);
	memset(w->raw + FAKE_GUARD, 0, len);
	w->p = w->raw + FAKE_GUARD;
	w->phys = phys;
	w->len = len;
	w->flags = flags;
	w->live = true;
	g_fvmm.io_maps++;
	return (w->p);
}

static bool	guard_ok(const t_fwin *w)
{
	size_t	i;

	i = 0;
	while (i < FAKE_GUARD)
	{
		if (w->raw[i] != 0xa5 || w->p[w->len + i] != 0xa5)
			return (false);
		i++;
	}
	return (true);
}

void	vmm_io_unmap(void *virt, size_t len)
{
	int	i;

	i = 0;
	while (i < FAKE_WIN)
	{
		if (g_fvmm.win[i].live && g_fvmm.win[i].p == virt
			&& g_fvmm.win[i].len == len)
		{
			if (!guard_ok(&g_fvmm.win[i]))
				g_fvmm.guard_broken = true;
			free(g_fvmm.win[i].raw);
			g_fvmm.win[i].live = false;
			g_fvmm.io_unmaps++;
			return ;
		}
		i++;
	}
	g_fvmm.bad_unmaps++;
}
