#include "ws_core.h"

bool	wgl_translucent(const t_wwin *w)
{
	if (!(w->style & (WS_APPBAR | WS_POPUP)) || wh_decorated(w->style))
		return (false);
	if (w->content.px == NULL || w->content.w <= 0 || w->content.h <= 0)
		return (false);
	return ((w->content.px[0] >> 24) != 0xffu);
}

void	wgl_backdrop(const t_wtable *t, int slot, t_surface *s)
{
	const t_wwin	*w;

	w = &t->w[slot];
	if (!wgl_translucent(w))
		return ;
	if (w->style & WS_APPBAR)
		luna_taskbar(s, w->rect);
	else
		luna_glass(s, w->rect, LUNA_GLASS_LAUNCHER);
}

int32_t	wgl_bands(const t_wtable *t, int slot, t_rect *out)
{
	t_lunawin	lw;

	if (slot < 0 || slot >= WS_WIN_MAX || !wf_visible(&t->w[slot]))
		return (0);
	if (wgl_translucent(&t->w[slot]))
	{
		out[0] = t->w[slot].rect;
		return (1);
	}
	if (!wh_decorated(t->w[slot].style))
		return (0);
	wh_lunawin(t, slot, &lw);
	return (luna_window_glass_bands(&lw, out));
}

static bool	absorb(const t_wtable *t, int slot, t_rect *r)
{
	t_rect	b[2];
	int32_t	n;
	int32_t	i;
	bool	grown;

	grown = false;
	n = wgl_bands(t, slot, b);
	i = 0;
	while (i < n)
	{
		b[i] = rect_intersect(b[i], t->screen);
		if (!rect_empty(rect_intersect(b[i], *r)) && !wr_inside(*r, b[i]))
		{
			*r = rect_union(*r, b[i]);
			grown = true;
		}
		i++;
	}
	return (grown);
}

static bool	grow(const t_wtable *t, t_rect *r)
{
	uint32_t	i;
	bool		grown;

	grown = false;
	i = 0;
	while (i < t->nz && i < WS_WIN_MAX)
	{
		if (absorb(t, t->z[i], r))
			grown = true;
		i++;
	}
	return (grown);
}

t_rect	wgl_extend_n(const t_wtable *t, t_rect r, uint32_t passes)
{
	uint32_t	i;

	r = rect_intersect(r, t->screen);
	if (rect_empty(r))
		return (r);
	i = 0;
	while (i < passes)
	{
		if (!grow(t, &r))
			return (r);
		i++;
	}
	return (t->screen);
}

t_rect	wgl_extend(const t_wtable *t, t_rect r)
{
	return (wgl_extend_n(t, r, WS_WIN_MAX + 1));
}
