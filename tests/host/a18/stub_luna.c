#include "help.h"

t_rect	luna_window_client(const t_lunawin *w)
{
	if (w->maximized)
		return (rect_make(w->outer.x, w->outer.y + 30, w->outer.w,
				w->outer.h - 30));
	return (rect_make(w->outer.x + 4, w->outer.y + 30, w->outer.w - 8,
			w->outer.h - 34));
}

static int32_t	side(int32_t a, int32_t b, int32_t lim)
{
	if (a < lim)
		return (-1);
	if (b < lim)
		return (1);
	return (0);
}

static uint32_t	edge(const t_lunawin *w, t_point p)
{
	static const uint32_t	ht[3][3] = {{HT_TOPLEFT, HT_TOP, HT_TOPRIGHT},
	{HT_LEFT, HT_NOWHERE, HT_RIGHT},
	{HT_BOTTOMLEFT, HT_BOTTOM, HT_BOTTOMRIGHT}};
	int32_t					d[4];
	int32_t					sx;
	int32_t					sy;

	if (!(w->style & WS_SIZEBOX) || w->maximized)
		return (HT_NOWHERE);
	d[0] = p.x - w->outer.x;
	d[1] = w->outer.x + w->outer.w - 1 - p.x;
	d[2] = p.y - w->outer.y;
	d[3] = w->outer.y + w->outer.h - 1 - p.y;
	sx = side(d[0], d[1], 4);
	sy = side(d[2], d[3], 4);
	if (sx != 0 && sy == 0)
		sy = side(d[2], d[3], 16);
	else if (sy != 0 && sx == 0)
		sx = side(d[0], d[1], 16);
	return (ht[sy + 1][sx + 1]);
}

static uint32_t	caption(const t_lunawin *w, t_point p)
{
	int32_t	k;

	k = (w->outer.x + w->outer.w - 4 - p.x) / 23;
	if (p.y < w->outer.y + 5 || p.y >= w->outer.y + 26
		|| p.x >= w->outer.x + w->outer.w - 4)
		return (HT_CAPTION);
	if (k == 0 && (w->style & WS_SYSMENU))
		return (HT_CLOSE);
	if (k == 1 && (w->style & WS_MAXBOX))
		return (HT_MAXBUTTON);
	if (k == 2 && (w->style & WS_MINBOX))
		return (HT_MINBUTTON);
	return (HT_CAPTION);
}

uint32_t	luna_hit_test(const t_lunawin *w, t_point p)
{
	uint32_t	h;

	if (!rect_contains(w->outer, p))
		return (HT_NOWHERE);
	h = edge(w, p);
	if (h != HT_NOWHERE)
		return (h);
	if (rect_contains(luna_window_client(w), p))
		return (HT_CLIENT);
	return (caption(w, p));
}
