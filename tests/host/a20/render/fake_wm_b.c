#include "velum/err.h"
#include "render.h"

int	wmc_set_state(t_wmwin *w, uint32_t state)
{
	t_fwin	*f;

	f = fwm_by_id(w->id);
	if (!f)
		return (E_INVAL);
	f->state = state;
	return (0);
}

int	wmc_set_state_id(uint32_t window, uint32_t state)
{
	t_fwin	*f;

	g_fwm.state_id = window;
	g_fwm.state_value = state;
	f = fwm_by_id(window);
	if (!f)
		return (E_INVAL);
	f->state = state;
	return (0);
}

int	wmc_activate(uint32_t window)
{
	g_fwm.activated = window;
	return (0);
}

int	wmc_capture(t_wmwin *w, uint32_t on)
{
	if (on)
		g_fwm.captured = w->id;
	else if (g_fwm.captured == w->id)
		g_fwm.captured = 0;
	return (0);
}

int	wmc_subscribe(uint32_t on)
{
	g_fwm.subscribed = (on != 0);
	return (0);
}
