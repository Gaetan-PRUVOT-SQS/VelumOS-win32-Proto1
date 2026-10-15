#include "gfx_int.h"

void	gfx_job_run(const t_job *j, t_rowfn row)
{
	int64_t	y;
	int64_t	step;
	int64_t	left;
	bool	back;

	back = (uintptr_t)gfx_px_at(j->dst, j->d.x0, j->d.y0)
		> (uintptr_t)gfx_px_at(j->src, j->d.x0 + j->ox, j->d.y0 + j->oy);
	y = j->d.y0;
	step = 1;
	if (back)
	{
		y = j->d.y1 - 1;
		step = -1;
	}
	left = j->d.y1 - j->d.y0;
	while (left > 0)
	{
		row(gfx_px_at(j->dst, j->d.x0, y),
			gfx_px_at(j->src, j->d.x0 + j->ox, y + j->oy),
			(size_t)(j->d.x1 - j->d.x0));
		y += step;
		left--;
	}
}

void	gfx_blit(const t_blit *b)
{
	t_job	j;

	if (gfx_blit_job(&j, b))
		gfx_job_run(&j, gfx_span_move);
}

void	gfx_blit_alpha(const t_blit *b)
{
	t_job	j;

	if (gfx_blit_job(&j, b))
		gfx_job_run(&j, gfx_span_blend);
}

void	gfx_scroll(t_surface *s, t_rect r, t_point delta)
{
	t_job	j;
	t_box	area;

	area = gfx_box_clip(gfx_box_of(r), gfx_bounds(s));
	j.d = gfx_box_clip(area, gfx_box_make(area.x0 + delta.x,
				area.y0 + delta.y, area.x1 + delta.x, area.y1 + delta.y));
	j.d = gfx_box_clip(j.d, gfx_clipbox(s));
	j.dst = s;
	j.src = s;
	j.ox = -(int64_t)delta.x;
	j.oy = -(int64_t)delta.y;
	if (!gfx_box_empty(j.d))
		gfx_job_run(&j, gfx_span_move);
}
