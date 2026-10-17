#include "lt_fuzz.h"

uint32_t	ltf_rnd(uint64_t *seed, uint32_t n)
{
	*seed ^= *seed << 13;
	*seed ^= *seed >> 7;
	*seed ^= *seed << 17;
	return ((uint32_t)(*seed >> 32) % n);
}

static int64_t	sel(int64_t a, int64_t b, bool want_max)
{
	if ((a > b) == want_max)
		return (a);
	return (b);
}

t_rect	ltf_zone(t_rect r, t_rect clip, int32_t w, int32_t h)
{
	int64_t	b[4];
	t_rect	out;

	if (r.x < -LP_COORD_MAX || r.x > LP_COORD_MAX || r.y < -LP_COORD_MAX
		|| r.y > LP_COORD_MAX || r.w <= 0 || r.h <= 0 || clip.w <= 0
		|| clip.h <= 0)
		return (lp_rect(0, 0, 0, 0));
	b[0] = sel(sel(r.x, 0, true), clip.x, true);
	b[1] = sel(sel(r.y, 0, true), clip.y, true);
	b[2] = sel(sel((int64_t)r.x + r.w, w, false), (int64_t)clip.x + clip.w,
			false);
	b[3] = sel(sel((int64_t)r.y + r.h, h, false), (int64_t)clip.y + clip.h,
			false);
	if (b[2] <= b[0] || b[3] <= b[1])
		return (lp_rect(0, 0, 0, 0));
	out = lp_rect((int32_t)b[0], (int32_t)b[1], 0, 0);
	out.w = (int32_t)(b[2] - b[0]);
	out.h = (int32_t)(b[3] - b[1]);
	return (out);
}

static bool	pixel_ok(const t_lt *t, t_rect zone, t_point p, t_color back)
{
	t_color	c;

	c = t->s.px[(size_t)p.y * (size_t)t->s.stride + (size_t)p.x];
	if (lp_in(zone, p))
		return ((c >> 24) == 0xff);
	return (c == back);
}

bool	ltf_surface_ok(const t_lt *t, t_rect zone, t_color back)
{
	t_point	p;

	p.y = -1;
	while (++p.y < t->h)
	{
		p.x = -1;
		while (++p.x < t->w)
		{
			if (!pixel_ok(t, zone, p, back))
				return (false);
		}
	}
	return (lt_guard_ok(t) != 0);
}
