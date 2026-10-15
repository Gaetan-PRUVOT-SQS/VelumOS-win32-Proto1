#include "luna_int.h"

static const uint8_t	g_v[] = {0, 0, 6, 16, 12, 0, 254};
static const uint8_t	g_e[] = {0, 11, 10, 11, 10, 9, 9, 7, 7, 6, 3, 6, 1, 7,
	0, 9, 0, 13, 1, 15, 3, 16, 7, 16, 9, 15, 10, 14, 254};
static const uint8_t	g_l[] = {0, 0, 0, 16, 254};
static const uint8_t	g_u[] = {0, 6, 0, 13, 1, 15, 3, 16, 7, 16, 9, 15, 10,
	13, 255, 10, 6, 10, 16, 254};
static const uint8_t	g_m[] = {0, 6, 0, 16, 255, 0, 9, 1, 7, 3, 6, 4, 6, 5, 7,
	6, 9, 6, 16, 255, 6, 9, 7, 7, 9, 6, 10, 6, 11, 7, 12, 9, 12, 16, 254};
static const uint8_t	g_o[] = {4, 0, 8, 0, 11, 2, 12, 5, 12, 11, 11, 14, 8,
	16, 4, 16, 1, 14, 0, 11, 0, 5, 1, 2, 4, 0, 254};
static const uint8_t	g_s[] = {12, 3, 10, 1, 8, 0, 4, 0, 1, 2, 0, 4, 1, 6, 4,
	8, 8, 8, 11, 10, 12, 12, 11, 14, 8, 16, 4, 16, 2, 15, 0, 13, 254};
static const t_lglyph	g_letters[LP_LETTERS] = {{g_v, 12}, {g_e, 10},
{g_l, 0}, {g_u, 10}, {g_m, 12}, {g_o, 12}, {g_s, 12}};

static int32_t	mk_span(int32_t cells, int32_t h)
{
	return (cells * h * 55 / 1024);
}

int32_t	lp_wordmark_w(int32_t h)
{
	int32_t	cells;
	int32_t	i;

	h = lp_clamp(h, 0, LP_ICON_MAX);
	cells = LP_LETTER_GAP * (LP_LETTERS - 1);
	i = 0;
	while (i < LP_LETTERS)
	{
		cells += g_letters[i].w;
		i++;
	}
	return (mk_span(cells, h) + 9 * h / 64 + 1);
}

static int32_t	mk_scale(int32_t unit)
{
	return (lp_max(unit * 55 / 64 - 2, 1));
}

static t_point	mk_pt(const t_lmk *m, const uint8_t *p)
{
	int32_t	scale;

	scale = mk_scale(m->unit);
	return (lp_pt(m->org.x + p[0] * scale, m->org.y + p[1] * scale));
}

static void	letter_draw(t_surface *s, const t_lglyph *g, const t_lmk *m)
{
	const uint8_t	*p;
	t_lline			ln;
	bool			have;

	ln.w = 9 * m->unit / 4;
	ln.round = true;
	have = false;
	p = g->pts;
	while (p[0] != 254)
	{
		if (p[0] == 255)
			have = false;
		else
		{
			ln.b = mk_pt(m, p);
			if (have)
				lp_line(s, &ln, m->c);
			ln.a = ln.b;
			have = true;
		}
		p += 2 - (p[0] == 255);
	}
}

void	lp_wordmark(t_surface *s, t_rect r, t_color c)
{
	t_lmk	m;
	int32_t	i;

	if (!lp_ok(s) || r.h < 4 || r.h > LP_ICON_MAX
		|| !lp_pt_ok(lp_pt(r.x, r.y)))
		return ;
	m.unit = r.h;
	m.c = c;
	m.org = lp_pt(r.x * 16 + 9 * r.h / 8 + 16, r.y * 16 + 9 * r.h / 8 + 16);
	i = 0;
	while (i < LP_LETTERS)
	{
		letter_draw(s, &g_letters[i], &m);
		m.org.x += (g_letters[i].w + LP_LETTER_GAP) * r.h * 55 / 64;
		i++;
	}
}
