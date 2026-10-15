#include "display_int.h"
#include "velum/luna.h"

void	display_splash(uint32_t pct)
{
	if (pct > 100)
		pct = 100;
	if (!display_try_lock())
		return ;
	if (g_display.ready && g_display.state == DSP_SPLASH)
	{
		g_display.pct = pct;
		g_display.tick++;
		luna_boot_draw(&g_display.surf, pct, g_display.tick);
	}
	display_unlock();
}

void	display_boot_progress(uint32_t pct, const char *stage)
{
	(void)stage;
	if (g_display.ready && g_display.state == DSP_SPLASH)
		display_splash(pct);
}

void	display_boot_done(void)
{
	if (g_display.ready && g_display.state == DSP_SPLASH)
		display_state_set(DSP_OFF);
}
