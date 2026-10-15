#include "ws_core.h"

bool	wh_decorated(uint32_t style)
{
	if (!(style & WS_CAPTION))
		return (false);
	return (!(style & (WS_DESKTOP | WS_APPBAR | WS_POPUP | WS_FULLSCREEN)));
}

void	wh_lunawin(const t_wtable *t, int slot, t_lunawin *lw)
{
	const t_wwin	*w;

	w = &t->w[slot];
	lw->outer = w->rect;
	lw->style = w->style;
	lw->title = w->title;
	lw->icon = ICON_NONE;
	if (w->icon < ICON_IDS)
		lw->icon = (t_iconid)w->icon;
	lw->active = (t->active == slot);
	lw->maximized = (w->state == WSTATE_MAX);
	lw->hot = w->hot;
	lw->pressed = w->pressed;
}

t_rect	wh_client(const t_wtable *t, int slot)
{
	t_lunawin	lw;
	t_rect		c;

	if (!wh_decorated(t->w[slot].style))
		return (t->w[slot].rect);
	wh_lunawin(t, slot, &lw);
	c = luna_window_client(&lw);
	c = rect_intersect(c, t->w[slot].rect);
	if (c.w < 0 || c.h < 0)
		return (rect_make(t->w[slot].rect.x, t->w[slot].rect.y, 0, 0));
	return (c);
}

int	wh_window_at(const t_wtable *t, t_point p)
{
	uint32_t	i;
	int			slot;

	i = t->nz;
	while (i > 0)
	{
		i--;
		slot = t->z[i];
		if (wf_visible(&t->w[slot]) && rect_contains(t->w[slot].rect, p))
			return (slot);
	}
	return (-1);
}

uint32_t	wh_hit(const t_wtable *t, t_point p, int *slot)
{
	t_lunawin	lw;
	uint32_t	ht;
	uint32_t	i;

	i = t->nz;
	while (i > 0)
	{
		i--;
		*slot = t->z[i];
		if (!wf_visible(&t->w[*slot]) || !rect_contains(t->w[*slot].rect, p))
			continue ;
		if (!wh_decorated(t->w[*slot].style))
			return (HT_CLIENT);
		wh_lunawin(t, *slot, &lw);
		ht = luna_hit_test(&lw, p);
		if (ht != HT_NOWHERE)
			return (ht);
	}
	*slot = -1;
	return (HT_NOWHERE);
}
