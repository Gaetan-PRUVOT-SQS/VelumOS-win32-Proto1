#include "velum/err.h"
#include "velum/libk.h"
#include "wmc_int.h"

int	wmc_activate(uint32_t window)
{
	if (window == 0)
		return (E_INVAL);
	return (wmc_simple(WMC_ACTIVATE, window, 0));
}

int	wmc_subscribe(uint32_t on)
{
	return (wmc_simple(WMC_SUBSCRIBE, 0, on != 0));
}

t_handle	wmc_channel(void)
{
	return (g_wmc.chan);
}

int	wmc_destroy(t_wmwin *w)
{
	int	k;
	int	r;

	if (w == NULL || w->id == 0)
		return (E_INVAL);
	r = wmc_simple(WMC_DESTROY, w->id, 0);
	k = wmc_slot(w->id);
	if (k >= 0)
		g_wmc.wins[k] = NULL;
	wmc_detach(w);
	w->id = 0;
	return (r);
}

void	wmc_detach(t_wmwin *w)
{
	uint64_t	len;

	len = (uint64_t)w->surface.stride * (uint64_t)w->surface.h * 4;
	len = (len + WMC_PAGE - 1) & ~(WMC_PAGE - 1);
	if (w->surface.px != NULL)
		wmsys_unmap(w->surface.px, len);
	if (w->section != 0)
		wmsys_close(w->section);
	memset(&w->surface, 0, sizeof(w->surface));
	w->section = 0;
}
