#include "ctl_int.h"

int32_t	ctl_sb_axis(bool vertical, t_point p)
{
	if (vertical)
		return (p.y);
	return (p.x);
}

int32_t	ctl_sb_start(bool vertical, t_rect r)
{
	if (vertical)
		return (r.y);
	return (r.x);
}

uint32_t	ctl_sb_zone(const t_sblo *lo, t_point p)
{
	if (rect_contains(lo->dec, p))
		return (CTL_SBZ_DEC);
	if (rect_contains(lo->inc, p))
		return (CTL_SBZ_INC);
	if (rect_contains(lo->thumb, p))
		return (CTL_SBZ_THUMB);
	if (!rect_contains(lo->track, p))
		return (CTL_SBZ_NONE);
	if (ctl_sb_axis(lo->vertical, p) < ctl_sb_start(lo->vertical, lo->thumb))
		return (CTL_SBZ_PAGE_DEC);
	return (CTL_SBZ_PAGE_INC);
}

int32_t	ctl_sb_pos_from(const t_sblo *lo, const t_scrst *st,
		int32_t coord)
{
	int64_t	span;
	int64_t	rel;

	span = ctl_sb_len(lo->track, lo->vertical)
		- ctl_sb_len(lo->thumb, lo->vertical);
	if (span <= 0 || st->max <= 0)
		return (0);
	rel = (int64_t)coord - ctl_sb_start(lo->vertical, lo->track);
	if (rel <= 0)
		return (0);
	if (rel >= span)
		return (st->max);
	return ((int32_t)((rel * st->max + span / 2) / span));
}

bool	ctl_sb_set(t_scrst *st, int32_t pos)
{
	if (pos < 0)
		pos = 0;
	if (pos > st->max)
		pos = st->max;
	if (pos == st->pos)
		return (false);
	st->pos = pos;
	return (true);
}
