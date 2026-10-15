#include <stdlib.h>
#include "ref.h"

uint32_t	ref_cover(int64_t radius, int64_t u, int64_t v)
{
	const int64_t	side = 32;
	int64_t			k;
	int64_t			l;
	int64_t			dx;
	int64_t			n;

	n = 0;
	k = 0;
	while (k < side)
	{
		dx = 2 * side * (u - radius) + 2 * k + 1;
		l = 0;
		while (l < side)
		{
			if (dx * dx + (2 * side * (v - radius) + 2 * l + 1)
				* (2 * side * (v - radius) + 2 * l + 1)
				<= 4 * side * side * radius * radius)
				n++;
			l++;
		}
		k++;
	}
	return ((uint32_t)((n * 255 + side * side / 2) / (side * side)));
}

bool	hard_inside(t_rect r, const int32_t rad[4], int32_t x, int32_t y)
{
	int64_t	u[4];
	int64_t	v[4];
	int64_t	k;
	int64_t	dx;
	int64_t	dy;

	u[0] = (int64_t)x - r.x;
	v[0] = (int64_t)y - r.y;
	u[1] = (int64_t)r.x + r.w - 1 - x;
	v[1] = v[0];
	u[2] = u[1];
	v[2] = (int64_t)r.y + r.h - 1 - y;
	u[3] = u[0];
	v[3] = v[2];
	k = 0;
	while (k < 4)
	{
		dx = 2 * (rad[k] - u[k]) - 1;
		dy = 2 * (rad[k] - v[k]) - 1;
		if (u[k] < rad[k] && v[k] < rad[k]
			&& dx * dx + dy * dy > 4 * (int64_t)rad[k] * rad[k])
			return (false);
		k++;
	}
	return (true);
}

int32_t	ref_cap(int32_t radius, t_rect r)
{
	int32_t	cap;

	cap = r.w;
	if (r.h < cap)
		cap = r.h;
	cap = cap / 2;
	if (radius > cap)
		radius = cap;
	if (radius < 0)
		radius = 0;
	return (radius);
}

static t_color	corner_px(const t_scene *sc, t_rect r, int corner, t_point uv)
{
	int32_t	x;
	int32_t	y;

	x = r.x + uv.x;
	y = r.y + uv.y;
	if (corner & 1)
		x = r.x + r.w - 1 - uv.x;
	if (corner & 2)
		y = r.y + r.h - 1 - uv.y;
	return (sc->g.px[(size_t)y * (size_t)sc->w + (size_t)x]);
}

t_cstat	corner_stat(const t_scene *sc, t_rect r, int corner,
		int32_t rad)
{
	t_cstat	st;
	int64_t	err;
	t_point	uv;

	st = (t_cstat){0, 0, 0};
	uv.y = 0;
	while (uv.y < rad)
	{
		uv.x = 0;
		while (uv.x < rad)
		{
			st.area += corner_px(sc, r, corner, uv) & 255;
			err = llabs((long long)(corner_px(sc, r, corner, uv) & 255)
					- (long long)ref_cover(rad, uv.x, uv.y));
			st.abs_err += err;
			if (err > st.max_err)
				st.max_err = err;
			uv.x++;
		}
		uv.y++;
	}
	return (st);
}
