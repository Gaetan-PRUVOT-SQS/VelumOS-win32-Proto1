#include "dock_int.h"

bool	ld_dark(void)
{
	return (luna_theme_get() == LUNA_THEME_DARK);
}

t_color	ld_pick(t_color light, t_color dark)
{
	if (ld_dark())
		return (dark);
	return (light);
}

t_color	ld_accent(void)
{
	return (ld_pick(VT_ACCENT, VT_ACCENT_TEXT_DARK));
}
