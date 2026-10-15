#include "ws_core.h"

int32_t	wg_clamp(int32_t v, int32_t lo, int32_t hi)
{
	if (v > hi)
		v = hi;
	if (v < lo)
		v = lo;
	return (v);
}

void	wg_min_size(uint32_t style, int32_t *minw, int32_t *minh)
{
	t_lunametrics	m;

	*minw = 1;
	*minh = 1;
	if (!wh_decorated(style))
		return ;
	luna_metrics(&m);
	*minw = WS_MINTRACK_W;
	*minh = m.caption_h + m.frame_bottom;
	if (*minh < 1)
		*minh = 1;
}

t_rect	wg_clamp_pos(const t_wtable *t, uint32_t style, t_rect r)
{
	t_lunametrics	m;
	int32_t			keep;
	t_rect			s;

	s = t->screen;
	if (!wh_decorated(style))
	{
		r.x = wg_clamp(r.x, s.x, s.x + s.w - r.w);
		r.y = wg_clamp(r.y, s.y, s.y + s.h - r.h);
		return (r);
	}
	luna_metrics(&m);
	keep = WS_DRAG_KEEP;
	if (keep > r.w)
		keep = r.w;
	r.x = wg_clamp(r.x, s.x - (r.w - keep), s.x + s.w - keep);
	r.y = wg_clamp(r.y, t->work.y, t->work.y + t->work.h - m.caption_h);
	return (r);
}

t_rect	wg_normalize(const t_wtable *t, uint32_t style, t_rect r)
{
	int32_t	minw;
	int32_t	minh;

	if (style & (WS_DESKTOP | WS_FULLSCREEN))
		return (t->screen);
	wg_min_size(style, &minw, &minh);
	r.w = wg_clamp(r.w, minw, t->screen.w);
	r.h = wg_clamp(r.h, minh, t->screen.h);
	return (wg_clamp_pos(t, style, r));
}

t_rect	wg_maximized(const t_wtable *t)
{
	return (t->work);
}
