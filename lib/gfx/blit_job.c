#include "gfx_int.h"

bool	gfx_blit_job(t_job *j, const t_blit *b)
{
	int64_t	w;
	int64_t	h;
	t_box	s;

	if (b == NULL)
		return (false);
	w = gfx_min64(b->dr.w, b->sr.w);
	h = gfx_min64(b->dr.h, b->sr.h);
	if (w <= 0 || h <= 0)
		return (false);
	j->dst = b->dst;
	j->src = b->src;
	j->ox = (int64_t)b->sr.x - b->dr.x;
	j->oy = (int64_t)b->sr.y - b->dr.y;
	j->d = gfx_box_make(b->dr.x, b->dr.y, (int64_t)b->dr.x + w,
			(int64_t)b->dr.y + h);
	j->d = gfx_box_clip(j->d, gfx_clipbox(b->dst));
	s = gfx_bounds(b->src);
	j->d = gfx_box_clip(j->d, gfx_box_make(s.x0 - j->ox, s.y0 - j->oy,
				s.x1 - j->ox, s.y1 - j->oy));
	return (!gfx_box_empty(j->d));
}
