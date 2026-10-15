#include "lt_contrast.h"

static double	lk_pow24(double v)
{
	double	x;
	int		i;

	x = 1.0;
	i = 0;
	while (i < 64)
	{
		x = x - (x * x * x * x * x - v) / (5.0 * x * x * x * x);
		i++;
	}
	return (v * v * x * x);
}

static double	lk_lum(t_color c)
{
	static const double	k[3] = {0.0722, 0.7152, 0.2126};
	double				v;
	double				sum;
	int					i;

	sum = 0.0;
	i = 0;
	while (i < 3)
	{
		v = (double)((c >> (8 * i)) & 0xff) / 255.0;
		if (v <= 0.04045)
			v = v / 12.92;
		else
			v = lk_pow24((v + 0.055) / 1.055);
		sum += k[i] * v;
		i++;
	}
	return (sum);
}

double	lk_ratio(t_color a, t_color b)
{
	double	la;
	double	lb;

	la = lk_lum(a);
	lb = lk_lum(b);
	if (la < lb)
		return ((lb + 0.05) / (la + 0.05));
	return ((la + 0.05) / (lb + 0.05));
}

double	lk_worst(const t_surface *s, t_rect r, t_color text)
{
	double	worst;
	double	cur;
	t_point	p;

	worst = 21.0;
	r = lp_isect(r, lp_rect(0, 0, s->w, s->h));
	if (r.w <= 0 || r.h <= 0)
		return (0.0);
	p.y = r.y;
	while (p.y < r.y + r.h)
	{
		p.x = r.x;
		while (p.x < r.x + r.w)
		{
			cur = lk_ratio(text, gfx_get(s, p));
			if (cur < worst)
				worst = cur;
			p.x++;
		}
		p.y++;
	}
	return (worst);
}

t_rect	lk_band(t_rect r, int32_t line)
{
	t_rect	b;

	b = r;
	b.h = line + 2 * LK_SLACK;
	if (b.h > r.h)
		b.h = r.h;
	b.y = r.y + (r.h - b.h) / 2;
	return (b);
}
