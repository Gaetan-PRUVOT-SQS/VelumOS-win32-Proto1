#include "luna_int.h"

static t_color	lane(const uint32_t *c, const int32_t *w, int32_t sh)
{
	uint32_t	v;

	v = 0;
	v += ((c[0] >> sh) & 0xff) * (uint32_t)w[0];
	v += ((c[1] >> sh) & 0xff) * (uint32_t)w[1];
	v += ((c[2] >> sh) & 0xff) * (uint32_t)w[2];
	v += ((c[3] >> sh) & 0xff) * (uint32_t)w[3];
	return ((v >> 16) & 0xff);
}

t_color	lp_gb_sample(const t_lgb *o, int32_t x, int32_t y)
{
	int32_t		sx;
	int32_t		sy;
	int32_t		w[4];
	uint32_t	c[4];
	int32_t		i[4];

	sx = (x - o->src.x) * 64 - 96;
	sy = (y - o->src.y) * 64 - 96;
	i[0] = lp_clamp(sx >> 8, 0, o->w - 1);
	i[1] = lp_clamp((sx >> 8) + 1, 0, o->w - 1);
	i[2] = lp_clamp(sy >> 8, 0, o->h - 1);
	i[3] = lp_clamp((sy >> 8) + 1, 0, o->h - 1);
	w[0] = (256 - (sx & 255)) * (256 - (sy & 255));
	w[1] = (sx & 255) * (256 - (sy & 255));
	w[2] = (256 - (sx & 255)) * (sy & 255);
	w[3] = (sx & 255) * (sy & 255);
	c[0] = o->a[i[2] * o->w + i[0]];
	c[1] = o->a[i[2] * o->w + i[1]];
	c[2] = o->a[i[3] * o->w + i[0]];
	c[3] = o->a[i[3] * o->w + i[1]];
	return (0xff000000 | (lane(c, w, 16) << 16) | (lane(c, w, 8) << 8)
		| lane(c, w, 0));
}

t_color	lp_saturate(t_color c, int32_t sat10)
{
	int32_t	ch[3];
	int32_t	lum;
	int32_t	k;

	ch[0] = (int32_t)((c >> 16) & 0xff);
	ch[1] = (int32_t)((c >> 8) & 0xff);
	ch[2] = (int32_t)(c & 0xff);
	lum = (ch[0] * 77 + ch[1] * 151 + ch[2] * 28) >> 8;
	k = -1;
	while (++k < 3)
		ch[k] = lp_clamp(lum + (ch[k] - lum) * sat10 / 10, 0, 255);
	return (0xff000000 | ((t_color)ch[0] << 16) | ((t_color)ch[1] << 8)
		| (t_color)ch[2]);
}
