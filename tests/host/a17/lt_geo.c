#include <stdio.h>
#include "lt_geo.h"

t_lunawin	lt_win(t_rect outer, uint32_t style, bool maximized)
{
	t_lunawin	w;

	w.outer = outer;
	w.style = style;
	w.title = "Test";
	w.icon = ICON_PROGRAM;
	w.active = true;
	w.maximized = maximized;
	w.hot = 0;
	w.pressed = 0;
	return (w);
}

bool	lt_rect_eq(t_rect a, t_rect b)
{
	return (a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h);
}

bool	lt_rect_in(t_rect a, t_rect b)
{
	if (a.w <= 0 || a.h <= 0)
		return (true);
	return (a.x >= b.x && a.y >= b.y && (int64_t)a.x + a.w <= (int64_t)b.x + b.w
		&& (int64_t)a.y + a.h <= (int64_t)b.y + b.h);
}

bool	lt_rect_disjoint(t_rect a, t_rect b)
{
	if (a.w <= 0 || a.h <= 0 || b.w <= 0 || b.h <= 0)
		return (true);
	return ((int64_t)a.x + a.w <= b.x || (int64_t)b.x + b.w <= a.x
		|| (int64_t)a.y + a.h <= b.y || (int64_t)b.y + b.h <= a.y);
}

const char	*lt_rect_str(t_rect r)
{
	static char	buf[4][64];
	static int	n;

	n = (n + 1) % 4;
	snprintf(buf[n], sizeof(buf[n]), "(%d,%d,%d,%d)", r.x, r.y, r.w, r.h);
	return (buf[n]);
}
