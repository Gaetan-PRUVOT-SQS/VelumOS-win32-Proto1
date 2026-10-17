#include "ctl_int.h"

static t_color	paint_part(t_surface *s, t_rect r, t_lunastate st)
{
	t_lunabtn	b;

	b.r = r;
	b.state = st;
	b.is_default = false;
	b.focus = false;
	luna_button(s, &b);
	return (luna_button_ink(&b));
}

static t_lunastate	part_state(const t_scrst *st, uint32_t zone, bool live)
{
	if (!live)
		return (LS_DISABLED);
	if (st->zone == zone)
		return (LS_PRESSED);
	return (LS_NORMAL);
}

static void	step_button(t_surface *s, t_rect r, int dir, t_lunastate state)
{
	t_color	ink;

	ink = paint_part(s, r, state);
	if (state == LS_PRESSED)
	{
		r.x++;
		r.y++;
	}
	ctl_sb_arrow(s, r, dir, ink);
}

void	ctl_sb_paint(t_surface *s, t_rect bar, const t_scrst *st, bool en)
{
	t_sblo	lo;
	bool	live;

	ctl_sb_layout(bar, st, &lo);
	live = en && st->max > 0;
	gfx_fill(s, lo.track, ctl_color_track());
	step_button(s, lo.dec, !lo.vertical * 2, part_state(st, CTL_SBZ_DEC, live));
	step_button(s, lo.inc, !lo.vertical * 2 + 1,
		part_state(st, CTL_SBZ_INC, live));
	if (!rect_empty(lo.thumb))
		paint_part(s, lo.thumb, part_state(st, CTL_SBZ_THUMB, live));
}
