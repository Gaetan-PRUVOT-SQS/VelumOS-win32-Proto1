#include "gfx_int.h"

static void	grad_row(uint32_t *p, size_t n, t_ramp *rp, int64_t first)
{
	gfx_ramp_seek(rp, first);
	while (n > 0)
	{
		gfx_store(p, gfx_ramp_color(rp));
		gfx_ramp_next(rp);
		p++;
		n--;
	}
}

static void	grad_horizontal(t_surface *s, const t_gradient *g, t_box b)
{
	t_ramp	rp;
	bool	opaque;
	int64_t	y;
	size_t	n;

	gfx_ramp_init(&rp, g->from, g->to, g->r.w);
	opaque = (g->from >> 24) == 255 && (g->to >> 24) == 255;
	n = (size_t)(b.x1 - b.x0);
	y = b.y0;
	while (y < b.y1)
	{
		if (opaque && y > b.y0)
			gfx_span_copy(gfx_px_at(s, b.x0, y), gfx_px_at(s, b.x0, b.y0), n);
		else
			grad_row(gfx_px_at(s, b.x0, y), n, &rp, b.x0 - g->r.x);
		y++;
	}
}

static void	grad_vertical(t_surface *s, const t_gradient *g, t_box b)
{
	t_ramp	rp;
	int64_t	y;

	gfx_ramp_init(&rp, g->from, g->to, g->r.h);
	gfx_ramp_seek(&rp, b.y0 - g->r.y);
	y = b.y0;
	while (y < b.y1)
	{
		gfx_span_color(gfx_px_at(s, b.x0, y), (size_t)(b.x1 - b.x0),
			gfx_ramp_color(&rp));
		gfx_ramp_next(&rp);
		y++;
	}
}

void	gfx_gradient(t_surface *s, const t_gradient *g)
{
	t_box	b;

	if (g == NULL)
		return ;
	b = gfx_box_clip(gfx_box_of(g->r), gfx_clipbox(s));
	if (gfx_box_empty(b))
		return ;
	if (g->horizontal)
		grad_horizontal(s, g, b);
	else
		grad_vertical(s, g, b);
}
