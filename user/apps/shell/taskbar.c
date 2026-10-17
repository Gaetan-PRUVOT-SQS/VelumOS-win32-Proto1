#include "velum/err.h"
#include "../common/layout.h"
#include "taskbar.h"

static int	metrics_ok(const t_tbmetrics *m)
{
	int32_t	dock_h;

	if ((uint32_t)m->w - 1u >= TB_DIM_MAX || (uint32_t)m->h - 1u >= TB_DIM_MAX
		|| (uint32_t)m->margin > TB_DIM_MAX || (uint32_t)m->pad > TB_DIM_MAX
		|| (uint32_t)m->gap > TB_SIDE_GAP_MAX
		|| (uint32_t)m->start_d - 1u >= TB_DIM_MAX
		|| (uint32_t)m->tray_w - 1u >= TB_DIM_MAX
		|| (uint32_t)m->tray_h - 1u >= TB_DIM_MAX
		|| (uint32_t)m->task_h - 1u >= TB_DIM_MAX)
		return (0);
	dock_h = m->h - 2 * m->margin;
	return (dock_h >= m->start_d && dock_h >= m->tray_h
		&& dock_h >= m->task_h);
}

int	tb_regions(const t_tbmetrics *m, t_tbregions *out)
{
	int32_t	dock_h;
	int32_t	strip_x;
	int32_t	tray_x;

	if (!m || !out || !metrics_ok(m))
		return (E_INVAL);
	dock_h = m->h - 2 * m->margin;
	strip_x = m->margin + m->pad + m->start_d + m->gap + TB_STRIP_PAD_L;
	tray_x = m->w - m->margin - m->pad - m->tray_w;
	if (tray_x - m->gap - strip_x < 0)
		return (E_RANGE);
	out->start = lay_rect(m->margin + m->pad,
			m->margin + (dock_h - m->start_d) / 2, m->start_d, m->start_d);
	out->strip = lay_rect(strip_x, m->margin + (dock_h - m->task_h) / 2,
			tray_x - m->gap - strip_x, m->task_h);
	out->tray = lay_rect(tray_x, m->margin + (dock_h - m->tray_h) / 2,
			m->tray_w, m->tray_h);
	return (0);
}

static int32_t	fit_count(int32_t width, uint32_t count)
{
	int32_t	shown;

	shown = (width + TB_GAP) / (TB_BTN_MIN_W + TB_GAP);
	if (shown > (int32_t)count)
		shown = (int32_t)count;
	if (shown > TASK_MAX)
		shown = TASK_MAX;
	return (shown);
}

int	tb_layout(t_rect strip, uint32_t count, t_rect *out)
{
	int32_t	shown;
	int32_t	base;
	int32_t	extra;
	int32_t	x;
	int32_t	i;

	shown = fit_count(strip.w, count);
	if (shown <= 0 || strip.h <= 0)
		return (0);
	base = (strip.w - (shown - 1) * TB_GAP) / shown;
	extra = (strip.w - (shown - 1) * TB_GAP) % shown;
	if (base >= TB_BTN_MAX_W)
	{
		base = TB_BTN_MAX_W;
		extra = 0;
	}
	x = strip.x;
	i = 0;
	while (i < shown)
	{
		out[i] = lay_rect(x, strip.y, base + (int32_t)(i < extra), strip.h);
		x += out[i].w + TB_GAP;
		i++;
	}
	return (shown);
}

int32_t	tb_hit(const t_rect *r, uint32_t n, int32_t x, int32_t y)
{
	uint32_t	i;

	i = 0;
	while (i < n)
	{
		if (lay_contains(r[i], x, y))
			return ((int32_t)i);
		i++;
	}
	return (-1);
}
