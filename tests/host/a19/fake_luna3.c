#include "fake.h"

void	luna_menu_panel(t_surface *s, t_rect r)
{
	fake_log_add(s, FK_MENU_PANEL, r, 0);
	gfx_fill(s, r, 0xfffcfcfc);
}

void	luna_menu_item(t_surface *s, t_rect r, t_lunastate st)
{
	fake_log_add(s, FK_MENU_ITEM, r, (int)st);
	gfx_fill(s, r, fake_menu_hot_bg());
}

t_color	fake_menu_hot_bg(void)
{
	return (gfx_lerp(0xfffcfcfc, 0xff316ac5, 31));
}

t_color	luna_button_ink(const t_lunabtn *b)
{
	if (b == NULL)
		return (0xff000000);
	if (b->state == LS_DISABLED)
		return (0xff808080);
	if (b->is_default || b->state == LS_DEFAULT)
		return (0xffffffff);
	return (0xff000000);
}

t_color	luna_color_accent(void)
{
	return (0xff0f7d83);
}
