#include <string.h>
#include "harness.h"
#include "fake_sb.h"

void	fake_check_layout(t_rect bar, t_scrst st)
{
	t_sblo	lo;
	bool	v;
	int		len;

	ctl_sb_layout(bar, &st, &lo);
	v = lo.vertical;
	len = ctl_sb_len(bar, v);
	h_eq_i64("dec au debut", ctl_sb_start(v, lo.dec), ctl_sb_start(v, bar));
	h_eq_i64("piste juste apres dec", ctl_sb_start(v, lo.track),
		ctl_sb_start(v, lo.dec) + ctl_sb_len(lo.dec, v));
	h_eq_i64("inc juste apres la piste", ctl_sb_start(v, lo.inc),
		ctl_sb_start(v, lo.track) + ctl_sb_len(lo.track, v));
	h_eq_i64("inc a la fin", ctl_sb_start(v, lo.inc) + ctl_sb_len(lo.inc, v),
		ctl_sb_start(v, bar) + len);
	if (rect_empty(lo.thumb))
		return ;
	h_true(ctl_sb_start(v, lo.thumb) >= ctl_sb_start(v, lo.track),
		"pouce dans la piste (debut)");
	h_true(ctl_sb_start(v, lo.thumb) + ctl_sb_len(lo.thumb, v)
		<= ctl_sb_start(v, lo.track) + ctl_sb_len(lo.track, v),
		"pouce dans la piste (fin)");
}

void	fake_zone_counts(const t_sblo *lo, int *counts)
{
	t_point	p;

	memset(counts, 0, 6 * sizeof(int));
	p.y = 0;
	while (p.y < 120)
	{
		p.x = 0;
		while (p.x < 30)
		{
			counts[ctl_sb_zone(lo, p)]++;
			p.x++;
		}
		p.y++;
	}
}
