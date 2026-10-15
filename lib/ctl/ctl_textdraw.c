#include "ctl_int.h"

static void	run_text(t_surface *s, const t_textrun *run, t_color col, int off)
{
	t_textreq	rq;

	rq.font = run->font;
	rq.at.x = run->at.x + off;
	rq.at.y = run->at.y + off;
	rq.color = col;
	rq.text = run->buf;
	rq.len = run->cut;
	font_draw(s, &rq);
	if (!run->ellipsis)
		return ;
	rq.at.x += font_text_width(run->font, run->buf, run->cut);
	rq.text = "\xe2\x80\xa6";
	rq.len = 3;
	font_draw(s, &rq);
}

static void	run_underline(t_surface *s, const t_textrun *run, t_color col,
		int off)
{
	t_point	p;
	int32_t	cl;

	if (run->mn < 0 || run->mn >= run->cut)
		return ;
	p.x = run->at.x + off + font_text_width(run->font, run->buf, run->mn);
	p.y = run->at.y + off + run->font->ascent + 1;
	cl = (int32_t)ctl_u8_len(run->buf + run->mn, run->cut - run->mn);
	gfx_hline(s, p, font_text_width(run->font, run->buf + run->mn, cl), col);
}

static void	run_all(t_surface *s, const t_textrun *run, t_color col, int off)
{
	run_text(s, run, col, off);
	run_underline(s, run, col, off);
}

void	ctl_text_draw(t_surface *s, const t_ctltext *tx)
{
	t_textrun	run;

	if (!ctl_run_init(&run, tx))
		return ;
	ctl_run_fit(&run, tx);
	ctl_run_place(&run, tx);
	if (!tx->enabled)
		run_all(s, &run, ctl_color_shadow(), 1);
	run_all(s, &run, tx->color, 0);
}
