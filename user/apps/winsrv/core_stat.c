#include "ws_core.h"

uint64_t	wst_area(t_rect r)
{
	if (rect_empty(r))
		return (0);
	return ((uint64_t)r.w * (uint64_t)r.h);
}

uint64_t	wst_glass_px(const t_wtable *t, t_rect r)
{
	t_rect		b[2];
	uint64_t	sum;
	int32_t		n;
	int32_t		j;
	uint32_t	i;

	sum = 0;
	i = 0;
	while (i < t->nz && i < WS_WIN_MAX)
	{
		n = wgl_bands(t, t->z[i], b);
		j = 0;
		while (j < n)
		{
			sum += wst_area(rect_intersect(rect_intersect(b[j], t->screen),
						r));
			j++;
		}
		i++;
	}
	return (sum);
}

uint64_t	wst_paint_px(const t_wtable *t, t_rect r)
{
	const t_wwin	*w;
	uint64_t		sum;
	uint32_t		i;

	sum = 0;
	i = 0;
	while (i < t->nz && i < WS_WIN_MAX)
	{
		w = &t->w[t->z[i]];
		if (wf_visible(w))
			sum += wst_area(rect_intersect(rect_intersect(w->rect, t->screen),
						r));
		i++;
	}
	return (sum);
}

void	wst_note(t_wframestat *f, const t_wtable *t, t_rect dirty, t_rect comp)
{
	f->rects++;
	f->px_dirty += wst_area(dirty);
	f->px_comp += wst_area(comp);
	f->px_paint += wst_paint_px(t, comp);
	f->px_glass += wst_glass_px(t, comp);
}

static void	stat_add(t_wframestat *to, const t_wframestat *f)
{
	to->rects += f->rects;
	to->overflow += f->overflow;
	to->px_dirty += f->px_dirty;
	to->px_comp += f->px_comp;
	to->px_paint += f->px_paint;
	to->px_glass += f->px_glass;
	to->ns += f->ns;
}

void	wst_close(t_wmeter *m, const t_wframestat *f)
{
	m->last = *f;
	stat_add(&m->total, f);
	stat_add(&m->window, f);
	if (f->ns > m->ns_max)
		m->ns_max = f->ns;
	m->frames++;
	m->window_frames++;
}

bool	wst_due(t_wmeter *m, uint64_t now, uint64_t period)
{
	if (m->log_at == 0)
	{
		m->log_at = now + period;
		return (false);
	}
	if (now < m->log_at)
		return (false);
	if (m->window_frames == 0)
	{
		m->log_at = now + period;
		return (false);
	}
	return (true);
}

void	wst_rearm(t_wmeter *m, uint64_t now)
{
	m->window = (t_wframestat){0};
	m->window_frames = 0;
	m->ns_max = 0;
	m->log_at = now;
}
