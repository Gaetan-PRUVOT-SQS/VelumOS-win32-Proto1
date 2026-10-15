#include "ctl_int.h"

static int32_t	step_of(uint32_t zone, int32_t page)
{
	if (zone == CTL_SBZ_DEC)
		return (-1);
	if (zone == CTL_SBZ_INC)
		return (1);
	if (zone == CTL_SBZ_PAGE_DEC)
		return (-page);
	return (page);
}

static int	sb_down(const t_sbctx *x, const t_sblo *lo, t_point p)
{
	uint32_t	z;

	z = ctl_sb_zone(lo, p);
	if (z == CTL_SBZ_NONE)
		return (CTL_SBR_NONE);
	x->r->capture = x->c;
	x->st->zone = z;
	ctl_dirty_add(x->r, x->c->rect);
	if (z == CTL_SBZ_THUMB)
	{
		x->st->grab = ctl_sb_axis(lo->vertical, p)
			- ctl_sb_start(lo->vertical, lo->thumb);
		return (CTL_SBR_USED);
	}
	if (ctl_sb_set(x->st, x->st->pos + step_of(z, x->st->page)))
		return (CTL_SBR_MOVED);
	return (CTL_SBR_USED);
}

static int	sb_drag(const t_sbctx *x, const t_sblo *lo, t_point p)
{
	int32_t	pos;

	pos = ctl_sb_pos_from(lo, x->st, ctl_sb_axis(lo->vertical, p)
			- x->st->grab);
	if (!ctl_sb_set(x->st, pos))
		return (CTL_SBR_USED);
	ctl_dirty_add(x->r, x->c->rect);
	return (CTL_SBR_MOVED);
}

static int	sb_up(const t_sbctx *x)
{
	x->st->zone = CTL_SBZ_NONE;
	x->st->grab = 0;
	x->r->capture = NULL;
	ctl_dirty_add(x->r, x->c->rect);
	return (CTL_SBR_USED);
}

int	ctl_sb_mouse(const t_sbctx *x, const t_mouseev *e)
{
	t_sblo	lo;
	bool	mine;

	ctl_sb_layout(x->bar, x->st, &lo);
	mine = (x->r->capture == x->c);
	if (e->kind == CTL_MEV_DOWN)
		return (sb_down(x, &lo, e->at));
	if (e->kind == CTL_MEV_MOVE && mine && x->st->zone == CTL_SBZ_THUMB)
		return (sb_drag(x, &lo, e->at));
	if (e->kind == CTL_MEV_UP && mine)
		return (sb_up(x));
	return (CTL_SBR_NONE);
}
