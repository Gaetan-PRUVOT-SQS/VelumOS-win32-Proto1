#include "ref.h"

static t_color	tap(const t_surface *src, t_pair p, int64_t xi, int64_t yi)
{
	int64_t	x;
	int64_t	y;
	int64_t	hi;

	hi = (int64_t)p.sr.x + p.sr.w;
	if (src->w < hi)
		hi = src->w;
	x = ref_clamp(p.sr.x + xi, ref_clamp(p.sr.x, 0, INT32_MAX), hi - 1);
	hi = (int64_t)p.sr.y + p.sr.h;
	if (src->h < hi)
		hi = src->h;
	y = ref_clamp(p.sr.y + yi, ref_clamp(p.sr.y, 0, INT32_MAX), hi - 1);
	return (src->px[y * src->stride + x]);
}

static t_color	mix4(const t_color px[4], const int64_t w[4])
{
	t_color	out;
	int64_t	v;
	int		ch;
	int		k;

	out = 0;
	ch = 0;
	while (ch < 4)
	{
		v = 32768;
		k = 0;
		while (k < 4)
		{
			v += (int64_t)((px[k] >> (8 * ch)) & 255) * w[k];
			k++;
		}
		out |= (t_color)(v >> 16) << (8 * ch);
		ch++;
	}
	return (out);
}

static t_color	sample(const t_surface *src, t_pair p, t_point at)
{
	t_axis	tx;
	t_axis	ty;
	t_color	px[4];
	int64_t	w[4];

	tx = ref_axis((int64_t)at.x - p.dr.x, p.sr.w, p.dr.w);
	ty = ref_axis((int64_t)at.y - p.dr.y, p.sr.h, p.dr.h);
	px[0] = tap(src, p, tx.i0, ty.i0);
	px[1] = tap(src, p, tx.i1, ty.i0);
	px[2] = tap(src, p, tx.i0, ty.i1);
	px[3] = tap(src, p, tx.i1, ty.i1);
	w[0] = (256 - tx.frac) * (256 - ty.frac);
	w[1] = tx.frac * (256 - ty.frac);
	w[2] = (256 - tx.frac) * ty.frac;
	w[3] = tx.frac * ty.frac;
	return (mix4(px, w));
}

void	scene_smooth(t_scene *sc, const t_surface *src, t_pair p)
{
	int32_t	x;
	int32_t	y;
	t_rect	vis;

	if (p.sr.w > 65536 || p.sr.h > 65536)
	{
		scene_scaled(sc, src, p);
		return ;
	}
	vis = rect_intersect(p.sr, rect_make(0, 0, src->w, src->h));
	y = 0;
	while (y < sc->h && p.dr.w > 0 && p.dr.h > 0 && !rect_empty(vis))
	{
		x = 0;
		while (x < sc->w)
		{
			if (in_rect(p.dr, x, y))
				scene_set(sc, x, y, sample(src, p, pt(x, y)));
			x++;
		}
		y++;
	}
}
