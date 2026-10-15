#include "luna_int.h"

int32_t	lp_text_len(const char *s)
{
	int32_t	n;

	n = 0;
	while (n < LP_TEXT_MAX && s[n] != '\0')
		n++;
	return (n);
}

static int32_t	tx_fit(const t_font *f, const t_ltext *t, int32_t len,
					int32_t *ell)
{
	int32_t	n;

	*ell = 0;
	if (font_text_width(f, t->text, len) <= t->box.w)
		return (len);
	*ell = font_text_width(f, LP_ELLIPSIS, 3);
	n = font_fit(f, t->text, t->box.w - *ell);
	return (lp_clamp(n, 0, len));
}

static int32_t	tx_x(const t_ltext *t, int32_t w)
{
	if (w >= t->box.w || t->align == LA_LEFT)
		return (t->box.x);
	if (t->align == LA_CENTER)
		return (t->box.x + (t->box.w - w) / 2);
	return (t->box.x + t->box.w - w);
}

static void	tx_draw(t_surface *s, const t_ltx *x)
{
	t_textreq	rq;

	rq.font = x->f;
	rq.at = x->p;
	rq.color = x->c;
	rq.text = x->text;
	rq.len = x->n;
	if (x->n > 0)
		font_draw(s, &rq);
	if (x->ell == 0)
		return ;
	rq.at.x = x->p.x + font_text_width(x->f, x->text, x->n);
	rq.text = LP_ELLIPSIS;
	rq.len = 3;
	font_draw(s, &rq);
}

void	lp_text(t_surface *s, const t_ltext *t)
{
	t_ltx	x;
	int32_t	ell;

	if (s == NULL || t == NULL || t->text == NULL || t->box.w <= 0)
		return ;
	x.f = font_get(t->font);
	if (x.f == NULL)
		return ;
	x.text = t->text;
	x.n = tx_fit(x.f, t, lp_text_len(t->text), &ell);
	x.ell = ell;
	x.p.x = tx_x(t, font_text_width(x.f, t->text, x.n) + ell);
	x.p.y = t->box.y + (t->box.h - x.f->height) / 2;
	if ((t->shadow >> 24) != 0)
	{
		x.c = t->shadow;
		x.p.x++;
		x.p.y++;
		tx_draw(s, &x);
		x.p.x--;
		x.p.y--;
	}
	x.c = t->color;
	tx_draw(s, &x);
}
