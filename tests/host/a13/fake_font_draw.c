#include "fakes.h"

static void	record(uint32_t cp, int32_t x, const t_textreq *rq)
{
	t_fdraw	*e;

	e = &g_ffont.log[g_ffont.glyphs % FAKE_LOG];
	e->cp = cp;
	e->x = x;
	e->y = rq->at.y;
	e->color = rq->color;
	g_ffont.glyphs++;
}

static const char	*step(const char *p, const char *end, uint32_t *cp)
{
	const char	*q;

	q = p;
	*cp = font_utf8_next(&q, end);
	if (q <= p)
		q = p + 1;
	if (q > end)
		q = end;
	return (q);
}

void	font_draw(t_surface *dst, const t_textreq *rq)
{
	const char	*p;
	const char	*end;
	uint32_t	cp;
	int32_t		x;

	p = rq->text;
	end = p + rq->len;
	x = rq->at.x;
	g_ffont.calls++;
	while (p < end)
	{
		p = step(p, end, &cp);
		record(cp, x, rq);
		if (cp != ' ')
			gfx_fill(dst, rect_make(x + 1, rq->at.y + 2, 6, 10), rq->color);
		x += g_ffont.cell_w;
	}
}
