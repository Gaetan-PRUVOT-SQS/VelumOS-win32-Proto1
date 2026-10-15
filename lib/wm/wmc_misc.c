#include "velum/err.h"
#include "wmc_int.h"

int	wmc_set_state(t_wmwin *w, uint32_t state)
{
	if (w == NULL || state > WSTATE_HIDDEN)
		return (E_INVAL);
	return (wmc_simple(WMC_SET_STATE, w->id, state));
}

int	wmc_set_state_id(uint32_t window, uint32_t state)
{
	if (window == 0 || state > WSTATE_HIDDEN)
		return (E_INVAL);
	return (wmc_simple(WMC_SET_STATE, window, state));
}

int	wmc_set_cursor(t_wmwin *w, uint32_t cursor)
{
	if (w == NULL || cursor > CUR_HAND)
		return (E_INVAL);
	return (wmc_simple(WMC_SET_CURSOR, w->id, cursor));
}

int	wmc_set_icon(t_wmwin *w, uint32_t icon)
{
	if (w == NULL || icon >= ICON_IDS)
		return (E_INVAL);
	return (wmc_simple(WMC_SET_ICON, w->id, icon));
}

int	wmc_capture(t_wmwin *w, uint32_t on)
{
	if (w == NULL)
		return (E_INVAL);
	return (wmc_simple(WMC_CAPTURE, w->id, on != 0));
}
