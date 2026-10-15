#include <stdlib.h>
#include "velum/theme.h"
#include "harness.h"
#include "lt.h"
#include "luna_int.h"

#define SW 192
#define SH 128
#define TOLERANCE 0
#define SEED 0x1badf00du
#define NBACK 3
#define NRECT 8
#define NBLUR 5

static const t_rect	g_rect[NRECT] = {{40, 30, 100, 60}, {-20, -10, 90, 50},
{150, 100, 100, 80}, {10, 10, 1, 1}, {5, 5, 3, 2}, {0, 0, SW, SH},
{30, 50, 17, 33}, {20, 20, 150, 12}};
static const int32_t	g_blur[NBLUR] = {1, 4, 28, 34, 5000};

typedef struct s_ref
{
	uint32_t	*a;
	uint32_t	*b;
}	t_ref;

static uint32_t	next_rand(uint32_t *st)
{
	*st ^= *st << 13;
	*st ^= *st >> 17;
	*st ^= *st << 5;
	return (*st);
}

static void	back_noise(t_lt *t)
{
	uint32_t	st;
	int32_t		x;
	int32_t		y;

	st = SEED;
	y = -1;
	while (++y < t->h)
	{
		x = -1;
		while (++x < t->w)
			t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]
				= 0xff000000u | (next_rand(&st) & 0x00ffffffu);
	}
}

static void	back_gradient(t_lt *t)
{
	int32_t	x;
	int32_t	y;

	y = -1;
	while (++y < t->h)
	{
		x = -1;
		while (++x < t->w)
			t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]
				= 0xff000000u | ((uint32_t)(x * 255 / t->w) << 16)
				| ((uint32_t)(y * 255 / t->h) << 8)
				| (uint32_t)((x + y) * 255 / (t->w + t->h));
	}
}

static void	back_stripes(t_lt *t)
{
	static const t_color	pal[8] = {0xff000000, 0xffff0000, 0xff00ff00,
		0xff0000ff, 0xffffff00, 0xff00ffff, 0xffff00ff, 0xffffffff};
	int32_t					x;
	int32_t					y;

	y = -1;
	while (++y < t->h)
	{
		x = -1;
		while (++x < t->w)
			t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]
				= pal[((x / 7) + (y / 5)) & 7];
	}
}

static void	back_make(t_lt *t, int32_t kind)
{
	if (kind == 0)
		back_noise(t);
	else if (kind == 1)
		back_gradient(t);
	else
		back_stripes(t);
}

static void	ref_add(const uint32_t *in, const t_lgl *p, int32_t i, int32_t *s)
{
	uint32_t	c;

	c = in[lp_clamp(i, 0, p->n - 1) * p->step];
	s[0] += (int32_t)((c >> 16) & 0xff);
	s[1] += (int32_t)((c >> 8) & 0xff);
	s[2] += (int32_t)(c & 0xff);
}

static void	ref_sub(const uint32_t *in, const t_lgl *p, int32_t i, int32_t *s)
{
	uint32_t	c;

	c = in[lp_clamp(i, 0, p->n - 1) * p->step];
	s[0] -= (int32_t)((c >> 16) & 0xff);
	s[1] -= (int32_t)((c >> 8) & 0xff);
	s[2] -= (int32_t)(c & 0xff);
}

static void	ref_line(const uint32_t *in, uint32_t *out, const t_lgl *p)
{
	int32_t	sum[3];
	int32_t	win;
	int32_t	i;

	sum[0] = 0;
	sum[1] = 0;
	sum[2] = 0;
	win = 2 * p->r + 1;
	i = -p->r - 1;
	while (++i <= p->r)
		ref_add(in, p, i, sum);
	i = -1;
	while (++i < p->n)
	{
		out[i * p->step] = 0xff000000 | ((uint32_t)(sum[0] / win) << 16)
			| ((uint32_t)(sum[1] / win) << 8) | (uint32_t)(sum[2] / win);
		ref_add(in, p, i + p->r + 1, sum);
		ref_sub(in, p, i - p->r, sum);
	}
}

static void	ref_blur(t_lgb *o, int32_t r)
{
	t_lgl	p;
	int32_t	pass;
	int32_t	i;

	r = lp_clamp(r, 1, 4096);
	pass = -1;
	while (++pass < LX_PASSES)
	{
		p = (t_lgl){o->w, 1, r};
		i = -1;
		while (++i < o->h)
			ref_line(o->a + i * o->w, o->b + i * o->w, &p);
		p = (t_lgl){o->h, o->w, r};
		i = -1;
		while (++i < o->w)
			ref_line(o->b + i, o->a + i, &p);
	}
}

static uint32_t	ref_lane(const uint32_t *c, const int32_t *w, int32_t sh)
{
	uint32_t	v;

	v = 0;
	v += ((c[0] >> sh) & 0xff) * (uint32_t)w[0];
	v += ((c[1] >> sh) & 0xff) * (uint32_t)w[1];
	v += ((c[2] >> sh) & 0xff) * (uint32_t)w[2];
	v += ((c[3] >> sh) & 0xff) * (uint32_t)w[3];
	return ((v >> 16) & 0xff);
}

static t_color	ref_sample(const t_lgb *o, int32_t x, int32_t y)
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
	return (0xff000000 | (ref_lane(c, w, 16) << 16)
		| (ref_lane(c, w, 8) << 8) | ref_lane(c, w, 0));
}

static t_color	ref_saturate(t_color c, int32_t sat10)
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

static t_color	ref_pixel(const t_lglass *g, const t_lgb *o, t_point p)
{
	t_color	c;
	int32_t	spec_h;
	int32_t	a;

	c = ref_saturate(ref_sample(o, p.x, p.y), g->sat);
	c = lp_mix(c, g->fill | 0xff000000, g->fill >> 24);
	spec_h = g->r.h * VT_SPEC_END_PCT / 100;
	if (spec_h > 0 && p.y - g->r.y < spec_h)
	{
		a = (int32_t)(g->spec >> 24) * (spec_h - (p.y - g->r.y)) / spec_h;
		c = lp_mix(c, g->spec | 0xff000000, (uint32_t)a);
	}
	return (c);
}

static void	ref_paint(t_surface *s, const t_lglass *g, const t_lgb *o,
				t_rect v)
{
	t_point	p;
	int32_t	cov;
	t_color	c;

	p.y = v.y - 1;
	while (++p.y < v.y + v.h)
	{
		p.x = v.x - 1;
		while (++p.x < v.x + v.w)
		{
			if (lp_in(g->hole, p))
				continue ;
			cov = lp_rmask(g, p.x, p.y);
			if (cov == 0)
				continue ;
			c = ref_pixel(g, o, p);
			if (cov < 16)
				c = lp_mix(gfx_get(s, p), c, (uint32_t)(cov * 255 / 16));
			gfx_put(s, p, c | 0xff000000);
		}
	}
}

static void	ref_glass(t_surface *s, const t_lglass *in, const t_ref *buf)
{
	t_lglass	g;
	t_lgb		o;
	t_rect		src;
	t_rect		v;

	g = *in;
	g.r = lp_clean(in->r);
	v = lp_visible(s, g.r);
	src = lp_isect(g.r, lp_rect(0, 0, s->w, s->h));
	o.a = buf->a;
	o.b = buf->b;
	lp_gb_down(s, src, &o);
	ref_blur(&o, lp_max(g.blur / LX_SCALE, 1));
	ref_paint(s, &g, &o, v);
	lp_glass_rim(s, &g);
}

static int32_t	diff_max(const t_lt *x, const t_lt *y, int32_t *bad)
{
	size_t		i;
	int32_t		sh;
	int32_t		d;
	int32_t		m;

	m = 0;
	i = 0;
	while (i < x->count)
	{
		sh = 0;
		while (sh < 24)
		{
			d = (int32_t)((x->buf[i] >> sh) & 0xff)
				- (int32_t)((y->buf[i] >> sh) & 0xff);
			d = lp_max(d, -d);
			m = lp_max(m, d);
			if (d > TOLERANCE)
				(*bad)++;
			sh += 8;
		}
		i++;
	}
	return (m);
}

static void	check_one(t_lt *x, t_lt *y, const t_lglass *g, const char *what)
{
	uint32_t	*mem;
	t_ref		ref;
	int32_t		bad;
	int32_t		worst;

	mem = malloc(2 * (size_t)((SW + 3) / 4) * (size_t)((SH + 3) / 4)
			* sizeof(uint32_t));
	h_true(mem != NULL, "allocation du tampon");
	if (mem == NULL)
		return ;
	ref.a = mem;
	ref.b = mem + (size_t)((SW + 3) / 4) * (size_t)((SH + 3) / 4);
	luna_glass_scratch(mem, 2 * (size_t)((SW + 3) / 4)
		* (size_t)((SH + 3) / 4));
	lp_glass(&x->s, g);
	ref_glass(&y->s, g, &ref);
	bad = 0;
	worst = diff_max(x, y, &bad);
	h_eq_i64(what, bad, 0);
	h_true(worst <= TOLERANCE, "ecart maximal dans la tolerance");
	h_true(lt_guard_ok(x), "gardes intactes");
	luna_glass_scratch(NULL, 0);
	free(mem);
}

static void	prepare(t_lt *x, t_lt *y, int32_t back)
{
	back_make(x, back);
	back_make(y, back);
}

static void	kinds_and_rects(void)
{
	t_lt		x;
	t_lt		y;
	t_lglass	g;
	int32_t		k;
	int32_t		r;
	int32_t		b;

	h_true(lt_open(&x, SW, SH) == 0 && lt_open(&y, SW, SH) == 0,
		"allocation");
	b = -1;
	while (++b < NBACK * 2)
	{
		luna_theme_set(LUNA_THEME_LIGHT);
		if (b >= NBACK)
			luna_theme_set(LUNA_THEME_DARK);
		k = -1;
		while (++k < LUNA_GLASS_KINDS)
		{
			r = -1;
			while (++r < NRECT)
			{
				prepare(&x, &y, b % NBACK);
				lp_glass_make(&g, g_rect[r], (t_glasskind)k);
				check_one(&x, &y, &g, "rendu egal a la reference");
			}
		}
	}
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&x);
	lt_close(&y);
}

static void	tweak(t_lglass *g, int32_t i)
{
	g->fill = (g->fill & 0x00ffffff) | ((uint32_t)(i * 37 % 256) << 24);
	g->spec = (g->spec & 0x00ffffff) | ((uint32_t)(i * 91 % 256) << 24);
	g->sat = (i * 7) % 41;
	g->blur = g_blur[i % NBLUR];
	g->rad_top = (i * 5) % 40;
	g->rad_bot = (i * 11) % 40;
	g->hole = lp_rect(0, 0, 0, 0);
	if (i % 3 == 0)
		g->hole = lp_rect(g->r.x + 6, g->r.y + 4, g->r.w / 2, g->r.h / 2);
}

static void	params_and_hole(void)
{
	t_lt		x;
	t_lt		y;
	t_lglass	g;
	int32_t		i;

	h_true(lt_open(&x, SW, SH) == 0 && lt_open(&y, SW, SH) == 0,
		"allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	i = -1;
	while (++i < 60)
	{
		prepare(&x, &y, i % NBACK);
		lp_glass_make(&g, g_rect[i % NRECT], (t_glasskind)(i % LUNA_GLASS_KINDS));
		tweak(&g, i);
		check_one(&x, &y, &g, "parametres libres : rendu egal a la reference");
	}
	lt_close(&x);
	lt_close(&y);
}

int	main(void)
{
	h_begin("a17/glass_exact");
	h_run("neuf genres, deux themes, huit rectangles, trois fonds",
		kinds_and_rects);
	h_run("fill, spec, saturation, flou, coins et trou libres",
		params_and_hole);
	return (h_end());
}
