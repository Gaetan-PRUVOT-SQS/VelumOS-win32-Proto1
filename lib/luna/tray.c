#include "dock_int.h"

void	luna_tray_geo(t_rect tray, t_traygeo *out)
{
	int32_t	x;

	if (out == NULL)
		return ;
	tray = lp_clean(tray);
	x = tray.x + tray.w - LD_TRAY_PAD_R - VT_BADGE;
	out->badge = lp_rect(x, tray.y + (tray.h - VT_BADGE) / 2, VT_BADGE,
			VT_BADGE);
	x -= LD_TRAY_GAP + LD_TRAY_CLOCK_W;
	out->clock = lp_rect(x, tray.y, LD_TRAY_CLOCK_W, tray.h);
	x -= LD_TRAY_GAP + LD_TRAY_DOTS_W;
	out->dots = lp_rect(x, tray.y, LD_TRAY_DOTS_W, tray.h);
	x -= LD_TRAY_GAP + LD_TRAY_LANG_W;
	out->lang = lp_rect(x, tray.y, LD_TRAY_LANG_W, tray.h);
}

static void	tray_dots(t_surface *s, t_rect d)
{
	t_ldplate	p;
	int32_t		y;

	y = d.y + (d.h - 8) / 2;
	p = (t_ldplate){lp_rect(d.x, y, 7, 7), 4, ld_pick(VT_MUTED, VT_MUTED_D), 0};
	ld_plate(s, &p);
	p.r = lp_rect(d.x + 12, y, 7, 7);
	ld_plate(s, &p);
	p.r = lp_rect(d.x + 24, y, 14, 8);
	p.rad = 3;
	p.thick = 1;
	ld_ring(s, &p);
}

void	luna_tray(t_surface *s, t_rect r)
{
	t_ldplate	p;
	t_traygeo	g;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 0 || r.h <= 0)
		return ;
	p = (t_ldplate){r, VT_R_TRAY, ld_pick(0x59ffffff, 0x12ffffff), 0};
	ld_plate(s, &p);
	p.c = ld_pick(0xccffffff, 0x1fffffff);
	ld_rim(s, &p);
	luna_tray_geo(r, &g);
	tray_dots(s, g.dots);
	p.r = g.badge;
	p.rad = 11;
	p.c = ld_pick(3019898879U, 0x1fffffff);
	ld_plate(s, &p);
	p.c = ld_pick(0xffffffff, 0x33ffffff);
	ld_rim(s, &p);
}
