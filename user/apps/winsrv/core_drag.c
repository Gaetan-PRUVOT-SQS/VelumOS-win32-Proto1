#include "ws_core.h"

static uint32_t	edge_mask(uint32_t ht)
{
	if (ht == HT_LEFT)
		return (1);
	if (ht == HT_RIGHT)
		return (2);
	if (ht == HT_TOP)
		return (4);
	if (ht == HT_BOTTOM)
		return (8);
	if (ht == HT_TOPLEFT)
		return (5);
	if (ht == HT_TOPRIGHT)
		return (6);
	if (ht == HT_BOTTOMLEFT)
		return (9);
	if (ht == HT_BOTTOMRIGHT)
		return (10);
	return (0);
}

void	wd_begin(t_wdrag *d, int slot, uint32_t ht, t_point p)
{
	d->slot = slot;
	d->ht = ht;
	d->start = p;
	d->orig = rect_make(0, 0, 0, 0);
}

bool	wd_is_resize(uint32_t ht)
{
	return (edge_mask(ht) != 0);
}

t_rect	wd_target(const t_wdrag *d, const t_wtable *t, t_point p)
{
	int32_t		e[4];
	int32_t		mn[2];
	uint32_t	mask;

	if (!wd_is_resize(d->ht))
		return (wg_clamp_pos(t, t->w[d->slot].style, wr_offset(d->orig,
					p.x - d->start.x, p.y - d->start.y)));
	mask = edge_mask(d->ht);
	wg_min_size(t->w[d->slot].style, &mn[0], &mn[1]);
	e[0] = d->orig.x + (p.x - d->start.x) * (int32_t)(mask & 1);
	e[1] = d->orig.y + (p.y - d->start.y) * (int32_t)((mask >> 2) & 1);
	e[2] = d->orig.x + d->orig.w + (p.x - d->start.x)
		* (int32_t)((mask >> 1) & 1);
	e[3] = d->orig.y + d->orig.h + (p.y - d->start.y)
		* (int32_t)((mask >> 3) & 1);
	if (mask & 1)
		e[0] = wg_clamp(e[0], e[2] - t->screen.w, e[2] - mn[0]);
	if (mask & 2)
		e[2] = wg_clamp(e[2], e[0] + mn[0], e[0] + t->screen.w);
	if (mask & 4)
		e[1] = wg_clamp(e[1], wg_clamp(e[3] - t->screen.h, t->work.y,
					e[3] - mn[1]), e[3] - mn[1]);
	if (mask & 8)
		e[3] = wg_clamp(e[3], e[1] + mn[1], e[1] + t->screen.h);
	return (rect_make(e[0], e[1], e[2] - e[0], e[3] - e[1]));
}

uint32_t	wd_cursor(uint32_t ht)
{
	uint32_t	mask;

	mask = edge_mask(ht);
	if (mask == 1 || mask == 2)
		return (CUR_SIZE_WE);
	if (mask == 4 || mask == 8)
		return (CUR_SIZE_NS);
	if (mask == 5 || mask == 10)
		return (CUR_SIZE_NWSE);
	if (mask == 6 || mask == 9)
		return (CUR_SIZE_NESW);
	return (CUR_ARROW);
}
