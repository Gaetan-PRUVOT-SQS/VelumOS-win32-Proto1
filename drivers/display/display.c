#include "display_int.h"

t_display	g_display;

const t_dispinfo	*display_info(void)
{
	return (&g_display.info);
}

void	*display_fb(void)
{
	return (g_display.fb);
}

uint64_t	display_fb_phys(void)
{
	return (g_display.phys);
}

bool	display_ready(void)
{
	return (g_display.ready);
}

t_surface	*display_surface(void)
{
	if (!g_display.ready)
		return (NULL);
	return (&g_display.surf);
}
