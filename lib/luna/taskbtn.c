#include "dock_int.h"

static t_color	plate_color(t_lunastate st)
{
	if (st == LS_PRESSED || st == LS_DEFAULT || st == LS_FOCUSED)
		return (ld_pick(3019898879U, 0x29ffffff));
	if (st == LS_HOT)
		return (ld_pick(0x66ffffff, 0x14ffffff));
	return (ld_pick(0x38ffffff, 0x0dffffff));
}

static void	task_shadow(t_surface *s, t_rect r)
{
	t_ldplate	p;
	int32_t		k;

	k = 3;
	while (k > 0)
	{
		p.r = lp_rect(r.x - k + 1, r.y + 2 - k + 1, r.w + 2 * (k - 1),
				r.h + 2 * (k - 1));
		p.rad = VT_R_TASK + k - 1;
		p.c = ld_pick(0x0c282850, 0x1a000000);
		p.thick = 0;
		ld_plate(s, &p);
		k--;
	}
}

void	luna_task_button(t_surface *s, t_rect r, t_lunastate st)
{
	t_ldplate	p;
	bool		active;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 0 || r.h <= 0)
		return ;
	active = (st == LS_PRESSED || st == LS_DEFAULT || st == LS_FOCUSED);
	if (active)
		task_shadow(s, r);
	p = (t_ldplate){r, VT_R_TASK, plate_color(st), 0};
	ld_plate(s, &p);
	if (!active)
		return ;
	p.c = ld_pick(0xffffffff, 0x38ffffff);
	ld_rim(s, &p);
	if (r.w < VT_TASK_IND_W || r.h < 3 + 3)
		return ;
	p.r = lp_rect(r.x + (r.w - VT_TASK_IND_W) / 2, r.y + r.h - 6,
			VT_TASK_IND_W, 3);
	p.rad = 1;
	p.c = ld_accent();
	ld_plate(s, &p);
}
