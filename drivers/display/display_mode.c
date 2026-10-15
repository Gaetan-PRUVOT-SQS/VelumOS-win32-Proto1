#include "display_int.h"
#include "velum/err.h"

static int	dispi_lazy(void)
{
	int	rc;

	if (g_display.dispi.ready)
		return (E_OK);
	rc = dispi_hw_open(&g_display.dispi, g_display.phys);
	if (rc == E_OK)
		rc = dispi_detect(&g_display.dispi);
	if (rc < 0)
		return (E_NOTSUP);
	return (E_OK);
}

static uint32_t	mode_current(t_dispmode *out, uint32_t max)
{
	if (max == 0)
		return (1);
	if (!out)
		return (0);
	out[0].width = g_display.info.width;
	out[0].height = g_display.info.height;
	out[0].bpp = g_display.info.bpp;
	out[0].flags = DISP_MODE_CURRENT;
	return (1);
}

uint32_t	display_modes(t_dispmode *out, uint32_t max)
{
	t_modelist	l;
	uint32_t	n;

	if (!g_display.ready)
		return (0);
	mutex_lock(&g_display.lock);
	if (dispi_lazy() < 0)
		n = mode_current(out, max);
	else
	{
		l.out = out;
		l.max = max;
		l.cur_w = g_display.info.width;
		l.cur_h = g_display.info.height;
		n = dispi_list(&g_display.dispi, &l);
	}
	mutex_unlock(&g_display.lock);
	return (n);
}

static int	set_locked(uint32_t w, uint32_t h)
{
	int	rc;

	if (w == g_display.info.width && h == g_display.info.height)
		return (E_OK);
	rc = dispi_lazy();
	if (rc < 0)
		return (rc);
	if (!dispi_mode_ok(&g_display.dispi, w, h))
		return (E_INVAL);
	return (display_switch(w, h));
}

int	display_set_mode(uint32_t w, uint32_t h, uint32_t bpp)
{
	int	rc;

	if (!g_display.ready)
		return (E_NODEV);
	if (bpp != DISP_BPP)
		return (E_INVAL);
	mutex_lock(&g_display.lock);
	rc = set_locked(w, h);
	mutex_unlock(&g_display.lock);
	return (rc);
}
