#include "ctl_int.h"

static void	paint_frame(t_surface *s, t_rect r, bool en)
{
	t_rect	in;

	gfx_fill(s, r, ctl_color_back(en));
	luna_edit_frame(s, r, en);
	in = rect_make(r.x + CTL_FRAME, r.y + CTL_FRAME, r.w - 2 * CTL_FRAME,
			r.h - 2 * CTL_FRAME);
	gfx_set_clip(s, rect_intersect(s->clip, in));
}

static void	draw_seg(t_surface *s, t_edview *v, int32_t a, int32_t b)
{
	t_textreq	rq;

	if (b <= a)
		return ;
	rq.font = v->font;
	rq.at.x = v->org.x + font_text_width(v->font, v->disp, a);
	rq.at.y = v->org.y;
	rq.color = v->col;
	rq.text = v->disp + a;
	rq.len = b - a;
	font_draw(s, &rq);
}

static void	draw_runs(t_surface *s, t_edview *v, const t_ctl *c, bool sel)
{
	const t_edit	*e;
	int32_t			a;
	int32_t			b;

	e = c->priv;
	v->col = ctl_color_text(ctl_enabled(c));
	if (!sel)
	{
		draw_seg(s, v, 0, v->len);
		return ;
	}
	a = (int32_t)ctl_edt_dpos(c, ctl_edt_sel_from(e));
	b = (int32_t)ctl_edt_dpos(c, ctl_edt_sel_to(e));
	draw_seg(s, v, 0, a);
	draw_seg(s, v, b, v->len);
	gfx_fill(s, rect_make(v->org.x + font_text_width(v->font, v->disp, a),
			v->org.y, font_text_width(v->font, v->disp + a, b - a),
			v->font->height), luna_color_selection());
	v->col = ctl_color_selected_text();
	draw_seg(s, v, a, b);
}

void	ctl_edt_paint(t_ctlroot *r, t_ctl *c)
{
	t_edview		v;
	const t_edit	*e;
	bool			en;
	bool			focus;

	e = c->priv;
	en = ctl_enabled(c);
	paint_frame(r->surface, c->rect, en);
	if (!ctl_edt_view(c, &v))
		return ;
	v.org.x = c->rect.x + CTL_EDIT_INSET - e->scroll;
	v.org.y = c->rect.y + (c->rect.h - v.font->height) / 2;
	focus = en && r->focus == c;
	draw_runs(r->surface, &v, c, focus && ctl_edt_has_sel(e));
	if (focus)
		gfx_vline(r->surface, (t_point){v.org.x + ctl_edt_wv(&v, c, e->caret),
			v.org.y}, v.font->height, ctl_color_text(true));
}
