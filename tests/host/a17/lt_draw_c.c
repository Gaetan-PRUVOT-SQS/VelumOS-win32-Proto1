#include "lt_draw.h"

void	lt_d_scroll(t_surface *s, t_rect r)
{
	int	i;

	i = 0;
	while (i < 6)
	{
		luna_scrollbar(s, r, (t_lunastate)i);
		i++;
	}
}

void	lt_d_scroll_ex(t_surface *s, t_rect r)
{
	t_lunascroll	sb;

	sb.r = r;
	sb.total = 1000;
	sb.page = 100;
	sb.pos = 450;
	sb.up = LS_HOT;
	sb.down = LS_PRESSED;
	sb.thumb = LS_HOT;
	luna_scrollbar_ex(s, &sb);
	sb.total = INT32_MAX;
	sb.page = INT32_MIN;
	sb.pos = INT32_MAX;
	luna_scrollbar_ex(s, &sb);
	sb.r.w = r.h;
	sb.r.h = r.w;
	sb.total = 10;
	sb.page = 20;
	luna_scrollbar_ex(s, &sb);
}

void	lt_d_menu(t_surface *s, t_rect r)
{
	luna_menu_panel(s, r);
	luna_menu_item(s, r, LS_NORMAL);
	luna_menu_item(s, r, LS_HOT);
	luna_menu_item(s, r, LS_DISABLED);
}

void	lt_d_tooltip(t_surface *s, t_rect r)
{
	luna_tooltip(s, r);
}

void	lt_d_taskbar(t_surface *s, t_rect r)
{
	luna_taskbar(s, r);
	luna_tray(s, r);
}
