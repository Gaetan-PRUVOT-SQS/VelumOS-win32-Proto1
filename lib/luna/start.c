#include "dock_int.h"

static t_color	orb_color(t_lunastate st)
{
	if (st == LS_HOT)
		return (ld_pick(0xa6ffffff, 0x2effffff));
	return (ld_pick(0x8cffffff, 0x1affffff));
}

static void	orb_ring(t_surface *s, t_rect r)
{
	t_ldplate	p;

	p.r = lp_rect(r.x - 5, r.y - 5, r.w + 10, r.h + 10);
	p.rad = lp_min(r.w, r.h) / 2 + 5;
	p.c = ld_accent();
	p.thick = 2;
	ld_ring(s, &p);
}

void	luna_start_button(t_surface *s, t_rect r, t_lunastate st)
{
	t_ldplate	p;
	int32_t		logo;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 0 || r.h <= 0)
		return ;
	p = (t_ldplate){r, lp_min(r.w, r.h) / 2, orb_color(st), 0};
	ld_plate(s, &p);
	p.c = ld_pick(0xffffffff, 0x40ffffff);
	ld_rim(s, &p);
	logo = lp_min(r.w, r.h) * 32 / VT_LAUNCHER_D;
	luna_icon(s, lp_pt(r.x + (r.w - logo) / 2, r.y + (r.h - logo) / 2),
		ICON_LOGO, logo);
	if (st == LS_PRESSED)
		orb_ring(s, r);
}
