#include "dock_int.h"

static t_rect	dock_rect(t_rect bar)
{
	int32_t	h;
	int32_t	y;

	h = lp_min(VT_DOCK_H, bar.h);
	y = lp_max(bar.y, bar.y + bar.h - VT_DOCK_MARGIN - h);
	return (lp_rect(bar.x + VT_DOCK_MARGIN, y,
			lp_max(bar.w - 2 * VT_DOCK_MARGIN, 0), h));
}

void	luna_taskbar(t_surface *s, t_rect r)
{
	r = lp_clean(r);
	if (!lp_ok(s) || r.w <= 2 * VT_DOCK_MARGIN || r.h <= 0)
		return ;
	luna_glass(s, dock_rect(r), LUNA_GLASS_DOCK);
}

void	luna_taskbar_layout(t_rect bar, t_taskbarlayout *out)
{
	const t_lunametrics	*m;
	t_rect				d;
	int32_t				tray;

	if (out == NULL)
		return ;
	d = dock_rect(lp_clean(bar));
	m = lm_metrics();
	out->start = lp_rect(d.x + VT_DOCK_PAD_X, d.y + (d.h - m->start_w) / 2,
			m->start_w, m->start_w);
	tray = lp_clamp(d.w - 2 * VT_DOCK_PAD_X - m->start_w - VT_DOCK_GAP, 0,
			LD_TRAY_W);
	out->tray = lp_rect(d.x + d.w - VT_DOCK_PAD_X - tray,
			d.y + (d.h - VT_TRAY_H) / 2, tray, VT_TRAY_H);
	out->tasks = lp_rect(out->start.x + out->start.w + VT_DOCK_GAP
			+ LD_STRIP_PAD_L, d.y + (d.h - VT_TASK_H) / 2, 0, VT_TASK_H);
	out->tasks.w = lp_max(out->tray.x - VT_DOCK_GAP - out->tasks.x, 0);
}
