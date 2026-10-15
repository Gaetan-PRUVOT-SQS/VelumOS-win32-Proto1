#include "luna_int.h"

t_color	luna_color_accent(void)
{
	return (lp_pal()->accent);
}

t_color	luna_color_text(void)
{
	return (lp_pal()->ink);
}

t_color	luna_color_window(void)
{
	return (lp_pal()->opaque_content);
}

t_color	luna_color_selection(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (0xff0f7d83);
	return (lp_pal()->accent);
}
