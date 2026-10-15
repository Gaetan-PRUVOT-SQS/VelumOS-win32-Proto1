#include "ws_core.h"

static void	body(const t_wwin *w, t_rect cl, int32_t band, t_surface *s)
{
	t_blit		b;
	int32_t		rows;

	if (w->content.px == NULL)
		return ;
	rows = cl.h - band;
	if (rows < 0)
		rows = 0;
	b.dst = s;
	b.src = &w->content;
	b.sr = rect_make(0, 0, wg_clamp(w->content.w, 0, cl.w),
			wg_clamp(w->content.h, 0, rows));
	b.dr = rect_make(cl.x, cl.y, b.sr.w, b.sr.h);
	gfx_blit(&b);
}

static void	content(const t_wtable *t, int slot, t_surface *s)
{
	const t_wwin	*w;
	t_lunawin		lw;
	t_rect			cl;
	int32_t			band;

	w = &t->w[slot];
	cl = wh_client(t, slot);
	if (rect_empty(cl))
		return ;
	wh_lunawin(t, slot, &lw);
	band = 0;
	if (wh_decorated(w->style))
		band = luna_client_band(&lw);
	if (w->content.px == NULL || w->content.w < cl.w || w->content.h < cl.h)
		gfx_fill(s, cl, luna_color_window());
	body(w, cl, band, s);
	if (band > 0)
		luna_client_band_paint(s, &lw, &w->content, luna_color_window());
}

void	wc_window(const t_wtable *t, int slot, t_surface *s)
{
	t_lunawin	lw;

	if (wh_decorated(t->w[slot].style))
	{
		wh_lunawin(t, slot, &lw);
		luna_window_frame(s, &lw);
	}
	content(t, slot, s);
}

void	wc_compose(const t_wtable *t, t_surface *dst, t_wcursor c, t_rect d)
{
	t_surface	s;
	uint32_t	i;
	int			slot;

	s = *dst;
	gfx_set_clip(&s, d);
	if (rect_empty(s.clip))
		return ;
	gfx_fill(&s, s.clip, WS_DESK_COLOR);
	i = 0;
	while (i < t->nz)
	{
		slot = t->z[i];
		if (wf_visible(&t->w[slot])
			&& !rect_empty(rect_intersect(t->w[slot].rect, s.clip)))
			wc_window(t, slot, &s);
		i++;
	}
	wcur_draw(&s, c);
}
