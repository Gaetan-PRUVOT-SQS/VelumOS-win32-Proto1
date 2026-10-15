#include "lt_gal.h"

void	lt_gal_scrolls(t_surface *s, int32_t x, int32_t y)
{
	t_lunascroll	sb;
	int32_t			i;

	i = 0;
	while (i < 4)
	{
		sb.r = lp_rect(x + i * 24, y, 17, 150);
		sb.total = 100;
		sb.page = 25 + 15 * i;
		sb.pos = 20 * i;
		sb.up = LS_NORMAL;
		sb.down = (i == 1) * LS_HOT + (i == 2) * LS_PRESSED;
		sb.thumb = LS_NORMAL + (i == 3) * LS_HOT;
		luna_scrollbar_ex(s, &sb);
		i++;
	}
	luna_scrollbar(s, lp_rect(x + 110, y, 150, 17), LS_NORMAL);
	luna_scrollbar(s, lp_rect(x + 110, y + 24, 150, 17), LS_HOT);
	luna_scrollbar(s, lp_rect(x + 110, y + 48, 150, 17), LS_DISABLED);
}

void	lt_gal_misc(t_surface *s, int32_t x, int32_t y)
{
	int32_t	i;

	luna_edit_frame(s, lp_rect(x, y, 120, 21), true);
	luna_edit_frame(s, lp_rect(x + 130, y, 120, 21), false);
	luna_groupbox(s, lp_rect(x, y + 30, 250, 60), 50);
	i = 0;
	while (i < 4)
	{
		luna_progress(s, lp_rect(x, y + 100 + i * 20, 250, 15), i * 33);
		i++;
	}
	luna_menu_panel(s, lp_rect(x + 270, y, 140, 80));
	luna_menu_item(s, lp_rect(x + 271, y + 1, 138, 22), LS_NORMAL);
	luna_menu_item(s, lp_rect(x + 271, y + 23, 138, 22), LS_HOT);
	luna_menu_item(s, lp_rect(x + 271, y + 45, 138, 22), LS_DISABLED);
	luna_tooltip(s, lp_rect(x + 270, y + 100, 120, 20));
}
