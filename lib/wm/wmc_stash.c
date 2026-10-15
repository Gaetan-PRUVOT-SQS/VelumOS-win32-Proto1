#include "velum/libk.h"
#include "wmc_int.h"

static uint32_t	type_of(const t_wmcmsg *m)
{
	t_wmhdr	h;

	memcpy(&h, m->buf, sizeof(h));
	return (h.type);
}

void	wmc_stash_push(const t_wmcmsg *m)
{
	t_wmcmsg	*old;

	if (g_wmc.n == WMC_STASH)
	{
		if (type_of(m) == WMS_MOUSE || type_of(m) == WMS_PAINT)
			return ;
		old = &g_wmc.stash[g_wmc.head];
		if (old->handle != 0)
			wmsys_close(old->handle);
		g_wmc.head = (g_wmc.head + 1) % WMC_STASH;
		g_wmc.n--;
	}
	g_wmc.stash[(g_wmc.head + g_wmc.n) % WMC_STASH] = *m;
	g_wmc.n++;
}

bool	wmc_stash_pop(t_wmcmsg *m)
{
	if (g_wmc.n == 0)
		return (false);
	*m = g_wmc.stash[g_wmc.head];
	g_wmc.head = (g_wmc.head + 1) % WMC_STASH;
	g_wmc.n--;
	return (true);
}

int	wmc_slot(uint32_t id)
{
	int	k;

	k = 0;
	while (k < WM_WIN_PER_CLIENT)
	{
		if (id == 0 && g_wmc.wins[k] == NULL)
			return (k);
		if (id != 0 && g_wmc.wins[k] != NULL && g_wmc.wins[k]->id == id)
			return (k);
		k++;
	}
	return (-1);
}

void	wmc_on_message(t_wmcmsg *m)
{
	t_wmhdr	h;
	int		k;

	memcpy(&h, m->buf, sizeof(h));
	if (m->handle == 0)
		return ;
	k = -1;
	if (h.type == WMS_RESIZED && h.window != 0)
		k = wmc_slot(h.window);
	if (k >= 0)
		wmc_attach(g_wmc.wins[k], m);
	else
		wmsys_close(m->handle);
	m->handle = 0;
}
