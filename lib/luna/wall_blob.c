#include "luna_int.h"

static const int32_t	g_blob[LP_BLOBS][6] = {{120, 180, 550, 750, 62, 1},
{880, 120, 500, 700, 62, 2}, {720, 950, 650, 700, 62, 3},
{180, 1000, 550, 600, 60, 4}};

static void	blob_setup(t_lbc *b, const t_lwall *c, int32_t i)
{
	const int32_t	*p;
	int32_t			rx;
	int32_t			ry;

	p = g_blob[i];
	b->cx = p[0] * c->w / 1000;
	b->cy = p[1] * c->h / 1000;
	rx = lp_max(p[2] * c->w / 1000, 1);
	ry = lp_max(p[3] * c->h / 1000, 1);
	b->irx = (1 << 26) / rx;
	b->iry = (1 << 26) / ry;
	b->thr = p[4] * 1024 / 100;
	b->ithr = (255 << 16) / b->thr;
	b->col = lp_pal()->wall[p[5]];
}

static void	veil_setup(t_lvc *v, const t_lwall *c, int32_t i)
{
	static const int32_t	g[LP_VEILS][8] = {{300, 490, 400, 190, 1001,
		-213, 40, 0}, {800, 230, 350, 150, 974, 316, 50, 1}};

	v->cx = g[i][0] * c->w / 1000;
	v->cy = g[i][1] * c->h / 1000;
	v->a = lp_max(g[i][2] * c->w / 1000, 1);
	v->b = lp_max(g[i][3] * c->h / 1000, 1);
	v->co = g[i][4];
	v->si = g[i][5];
	v->sigma = g[i][6];
	v->down = g[i][7] == 0;
	v->peak = lp_pal()->veil_a;
	if (i == 1)
		v->peak = lp_pal()->veil_b;
}

void	lp_wall_setup(t_lwall *c, t_rect r, const t_surface *s)
{
	int32_t	i;

	c->w = lp_clamp(r.w, 8, LP_WALL_MAX);
	c->h = lp_clamp(r.h, 8, LP_WALL_MAX);
	c->ox = r.x;
	c->oy = r.y;
	c->vis = lp_visible(s, r);
	c->base = lp_pal()->wall[0];
	i = -1;
	while (++i < LP_BLOBS)
		blob_setup(&c->bc[i], c, i);
	i = -1;
	while (++i < LP_VEILS)
		veil_setup(&c->vc[i], c, i);
}

static int32_t	blob_alpha(const t_lbc *b, int32_t x, int32_t y)
{
	int64_t	nx;
	int64_t	ny;
	int64_t	d2;
	int64_t	d;

	nx = ((int64_t)(x - b->cx) * b->irx) >> 16;
	ny = ((int64_t)(y - b->cy) * b->iry) >> 16;
	d2 = nx * nx + ny * ny;
	if (d2 >= (int64_t)b->thr * b->thr)
		return (0);
	d = lp_isqrt((uint64_t)d2);
	return (lp_smooth256(256 - (int32_t)((d * b->ithr) >> 16) * 256 / 255)
		* 255 / 256);
}

t_color	lp_wall_blobs(const t_lwall *c, int32_t x, int32_t y)
{
	t_color	col;
	int32_t	i;
	int32_t	a;

	col = c->base;
	i = LP_BLOBS - 1;
	while (i >= 0)
	{
		a = blob_alpha(&c->bc[i], x, y);
		if (a > 0)
			col = lp_mix(col, c->bc[i].col, (uint32_t)a);
		i--;
	}
	return (col);
}
