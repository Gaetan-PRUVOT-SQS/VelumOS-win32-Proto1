#include "luna_int.h"

static void	bar_pill(t_surface *s, t_rect r, t_color top, t_color bot)
{
	t_lbox	b;
	t_lgrad	g;
	t_lstop	st[2];

	if (r.w < 1 || r.h < 1)
		return ;
	st[0].at = 0;
	st[0].c = top;
	st[1].at = lp_max(r.h - 1, 1);
	st[1].c = bot;
	g.st = st;
	g.n = 2;
	g.tint = 0;
	g.tint_t = 0;
	b.r = r;
	b.rad = lp_min(r.h, r.w) / 2;
	b.ring = NULL;
	b.nring = 0;
	b.fill = &g;
	lp_box(s, &b);
}

static void	bar_shine(t_surface *s, t_rect f, t_color c)
{
	int32_t	rad;

	rad = f.h / 2;
	if (f.h < 6 || f.w <= 2 * rad + 2)
		return ;
	lp_fill(s, lp_rect(f.x + rad, f.y + f.h / 4, f.w - 2 * rad, 1), c);
}

static t_rect	bar_span(const t_rect *r, uint32_t pct, uint32_t tick)
{
	int32_t	seg;
	int32_t	x;

	if (pct > 0)
	{
		if (pct > 100)
			pct = 100;
		return (lp_rect(r->x, r->y, r->w * (int32_t)pct / 100, r->h));
	}
	seg = r->h * 4;
	x = (int32_t)((tick * 3U) % (uint32_t)(r->w + seg)) - seg;
	return (lp_isect(lp_rect(r->x + x, r->y, seg, r->h), *r));
}

void	lp_boot_bar(t_surface *s, const t_rect *r, uint32_t pct, uint32_t tick)
{
	t_cpal	pal;
	t_color	track;
	t_rect	f;

	if (!lp_ok(s) || r == NULL || r->w < 2 || r->h < 2 || r->w > LG_DIM_MAX)
		return ;
	pal = lp_pal();
	track = (pal->ink & 0x00ffffffU) | 0x24000000U;
	bar_pill(s, *r, track, track);
	f = bar_span(r, pct, tick);
	bar_pill(s, f, pal->accent, pal->accent);
	bar_shine(s, f, lp_mix(pal->accent_hi, LC_WHITE, 128));
}
