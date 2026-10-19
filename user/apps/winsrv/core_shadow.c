#include "velum/theme.h"
#include "ws_core.h"

#define SH_BLUR_HALF 20
#define SH_OFFSET_Y 12
#define SH_ALPHA_LIGHT 0x47282850
#define SH_ALPHA_DARK 0x80000000

typedef struct s_shgeo
{
	t_rect	box;
	int32_t	rad2;
}	t_shgeo;

static int32_t	isqrt(int64_t v)
{
	int64_t	lo;
	int64_t	hi;
	int64_t	mid;

	lo = 0;
	hi = 1;
	while (hi * hi <= v)
		hi *= 2;
	while (hi - lo > 1)
	{
		mid = (lo + hi) / 2;
		if (mid * mid <= v)
			lo = mid;
		else
			hi = mid;
	}
	return ((int32_t)lo);
}

static int32_t	abs32(int32_t v)
{
	if (v < 0)
		return (-v);
	return (v);
}

static int32_t	sdist2(const t_shgeo *g, int32_t px, int32_t py)
{
	int32_t	qx;
	int32_t	qy;
	int32_t	ax;
	int32_t	ay;
	int32_t	inner;

	qx = abs32(2 * px + 1 - (2 * g->box.x + g->box.w)) - (g->box.w - g->rad2);
	qy = abs32(2 * py + 1 - (2 * g->box.y + g->box.h)) - (g->box.h - g->rad2);
	ax = wg_clamp(qx, 0, 1 << 20);
	ay = wg_clamp(qy, 0, 1 << 20);
	inner = qx;
	if (qy > inner)
		inner = qy;
	if (inner > 0)
		inner = 0;
	if (ax == 0 || ay == 0)
		return (ax + ay + inner - g->rad2);
	return (isqrt((int64_t)ax * ax + (int64_t)ay * ay) + inner - g->rad2);
}

static void	geo_make(t_shgeo *g, t_rect r, int32_t dy)
{
	int32_t	rad;

	rad = VT_R_WINDOW;
	if (rad * 2 > r.w)
		rad = r.w / 2;
	if (rad * 2 > r.h)
		rad = r.h / 2;
	g->box = rect_make(r.x, r.y + dy, r.w, r.h);
	g->rad2 = 2 * rad;
}

static uint32_t	falloff(int32_t sd2)
{
	int32_t		u;
	uint32_t	t;

	u = wg_clamp(sd2 + 2 * SH_BLUR_HALF, 0, 4 * SH_BLUR_HALF);
	t = (uint32_t)u * 256 / (uint32_t)(4 * SH_BLUR_HALF);
	return (256 - ((t * t * (768 - 2 * t)) >> 16));
}

static t_color	shadow_base(void)
{
	if (luna_theme_get() == LUNA_THEME_DARK)
		return (SH_ALPHA_DARK);
	return (SH_ALPHA_LIGHT);
}

static void	shadow_px(t_surface *s, t_point p, t_color base, uint32_t f)
{
	uint32_t	a;

	a = ((base >> 24) * f) >> 8;
	if (a == 0)
		return ;
	gfx_put(s, p, gfx_blend(gfx_get(s, p), (a << 24) | (base & 0x00ffffffu)));
}

static void	shadow_row(t_surface *s, const t_shgeo *win, const t_shgeo *sh,
				t_rect v)
{
	t_point	p;
	t_color	base;
	int32_t	core;

	base = shadow_base();
	core = win->box.y + win->rad2 / 2;
	p.y = v.y;
	p.x = v.x;
	while (p.x < v.x + v.w)
	{
		if (p.y >= core && p.y < win->box.y + win->box.h - win->rad2 / 2
			&& p.x >= win->box.x && p.x < win->box.x + win->box.w)
			p.x = win->box.x + win->box.w;
		else
		{
			if (sdist2(win, p.x, p.y) > 0)
				shadow_px(s, p, base, falloff(sdist2(sh, p.x, p.y)));
			p.x++;
		}
	}
}

bool	wsh_has(const t_wtable *t, int slot)
{
	const t_wwin	*w;

	if (slot < 0 || slot >= WS_WIN_MAX)
		return (false);
	w = &t->w[slot];
	if (!wf_visible(w) || !wh_decorated(w->style) || w->state == WSTATE_MAX)
		return (false);
	return (!rect_empty(w->rect));
}

t_rect	wsh_grow(t_rect r)
{
	return (rect_make(r.x - SH_BLUR_HALF, r.y + SH_OFFSET_Y - SH_BLUR_HALF,
			r.w + 2 * SH_BLUR_HALF, r.h + 2 * SH_BLUR_HALF));
}

t_rect	wsh_extent(const t_wtable *t, int slot)
{
	t_rect	r;
	t_rect	g;

	if (slot < 0 || slot >= WS_WIN_MAX)
		return (rect_make(0, 0, 0, 0));
	r = t->w[slot].rect;
	if (!wh_decorated(t->w[slot].style) || rect_empty(r))
		return (r);
	g = wsh_grow(r);
	return (rect_union(r, g));
}

void	wsh_paint(const t_wtable *t, int slot, t_surface *s)
{
	t_shgeo	win;
	t_shgeo	sh;
	t_rect	v;

	if (!wsh_has(t, slot))
		return ;
	v = rect_intersect(wsh_extent(t, slot), s->clip);
	if (rect_empty(v))
		return ;
	geo_make(&win, t->w[slot].rect, 0);
	geo_make(&sh, t->w[slot].rect, SH_OFFSET_Y);
	while (v.h > 0)
	{
		shadow_row(s, &win, &sh, rect_make(v.x, v.y, v.w, 1));
		v.y++;
		v.h--;
	}
}
