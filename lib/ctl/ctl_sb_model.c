#include "ctl_int.h"

bool	ctl_sb_vertical(t_rect r)
{
	return (r.h >= r.w);
}

int32_t	ctl_sb_len(t_rect r, bool vertical)
{
	if (vertical)
		return (r.h);
	return (r.w);
}

static t_rect	axis_rect(t_rect bar, bool vertical, int32_t off, int32_t len)
{
	if (vertical)
		return (rect_make(bar.x, bar.y + off, bar.w, len));
	return (rect_make(bar.x + off, bar.y, len, bar.h));
}

static t_rect	thumb_rect(const t_sblo *lo, const t_scrst *st)
{
	int64_t	span;
	int64_t	len;
	int64_t	off;

	span = ctl_sb_len(lo->track, lo->vertical);
	if (st->max <= 0 || span < CTL_THUMB_MIN)
		return (rect_make(0, 0, 0, 0));
	len = span * st->page / ((int64_t)st->max + st->page);
	if (len < CTL_THUMB_MIN)
		len = CTL_THUMB_MIN;
	off = (span - len) * st->pos / st->max;
	return (axis_rect(lo->track, lo->vertical, (int32_t)off, (int32_t)len));
}

void	ctl_sb_layout(t_rect bar, const t_scrst *st, t_sblo *lo)
{
	t_lunametrics	m;
	int32_t			len;
	int32_t			arrow;

	luna_metrics(&m);
	lo->vertical = ctl_sb_vertical(bar);
	len = ctl_sb_len(bar, lo->vertical);
	arrow = m.scroll_w;
	if (arrow > len / 2)
		arrow = len / 2;
	lo->dec = axis_rect(bar, lo->vertical, 0, arrow);
	lo->inc = axis_rect(bar, lo->vertical, len - arrow, arrow);
	lo->track = axis_rect(bar, lo->vertical, arrow, len - 2 * arrow);
	lo->thumb = thumb_rect(lo, st);
}
