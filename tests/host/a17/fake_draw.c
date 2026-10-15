#include "fake_font.h"

static t_fkspy	g_spy;

t_fkspy	*fk_spy(void)
{
	return (&g_spy);
}

static void	fk_note(const t_textreq *rq)
{
	t_fkrec	*r;

	if (g_spy.n > 0)
	{
		r = &g_spy.rec[g_spy.n - 1];
		if (r->box.x == rq->at.x + 1 && r->box.y == rq->at.y + 1)
			g_spy.n--;
	}
	if (g_spy.n >= FK_SPY_MAX)
		return ;
	r = &g_spy.rec[g_spy.n];
	g_spy.n++;
	r->box.x = rq->at.x;
	r->box.y = rq->at.y;
	r->box.w = font_text_width(rq->font, rq->text, rq->len);
	r->box.h = rq->font->height;
	r->c = rq->color;
}

void	fk_glyph(t_surface *s, uint32_t cp, const t_fkg *g)
{
	int32_t	x;
	int32_t	y;

	y = 0;
	while (y < 11)
	{
		x = 0;
		while (x < 5)
		{
			if (fk_pixel(cp, x, y))
			{
				gfx_put(s, (t_point){g->at.x + g->p->xoff + x,
					g->at.y + g->p->yoff + y}, g->c);
				if (g->p->bold)
					gfx_put(s, (t_point){g->at.x + g->p->xoff + x + 1,
						g->at.y + g->p->yoff + y}, g->c);
			}
			x++;
		}
		y++;
	}
}

void	font_draw(t_surface *dst, const t_textreq *rq)
{
	t_fkg		g;
	const char	*p;
	const char	*end;

	fk_note(rq);
	if (g_spy.mute)
		return ;
	g.p = fk_params(rq->font);
	g.at = rq->at;
	g.c = rq->color;
	p = rq->text;
	end = rq->text + rq->len;
	while (p < end)
	{
		fk_glyph(dst, fk_utf8(&p, end), &g);
		g.at.x += g.p->adv;
	}
}
