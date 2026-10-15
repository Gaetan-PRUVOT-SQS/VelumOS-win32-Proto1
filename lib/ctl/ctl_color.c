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
	if (enabled)
		return (0xffffffff);
	return (luna_color_window());
}

t_color	ctl_color_track(void)
{
	return (gfx_lerp(luna_color_window(), 0xffffffff, 128));
}
