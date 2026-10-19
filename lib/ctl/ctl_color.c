#include "velum/luna.h"
#include "velum/luna_colors.h"
#include "ctl_int.h"

t_color	ctl_color_text(bool enabled)
{
	if (enabled)
		return (luna_color_text());
	return (gfx_lerp(luna_color_window(), luna_color_text(), 120));
}

t_color	ctl_color_shadow(void)
{
	return (0xffffffff);
}

t_color	ctl_color_selected_text(void)
{
	return (0xffffffff);
}

t_color	ctl_color_back(bool enabled)
{
	if (!enabled)
		return (luna_color_window());
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (luna_color_panel());
	return (0xffffffff);
}

t_color	ctl_color_track(void)
{
	return (gfx_lerp(luna_color_window(), 0xffffffff, 128));
}
