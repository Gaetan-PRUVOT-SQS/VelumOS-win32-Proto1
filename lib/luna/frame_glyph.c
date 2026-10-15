#include "luna_int.h"

static void	glyph_box(t_surface *s, t_rect r, int32_t rad, t_color c)
{
	t_llayers	l;
	t_lgrad		g;
	t_lstop		st;

	lp_flat(&g, &st, c);
	l.r = r;
	l.rad[0] = rad;
	l.rad[1] = rad;
	l.rad[2] = rad;
	l.rad[3] = rad;
	l.ring = NULL;
	l.nring = 0;
	l.fill = &g;
	l.hole = lp_inset(r, 2);
	l.skip = 0;
	lp_layers(s, &l);
}

static void	glyph_restore(t_surface *s, t_point c, t_color col)
{
	lp_fill(s, lp_rect(c.x - 2, c.y - 4, 6, 2), col);
	lp_fill(s, lp_rect(c.x + 2, c.y - 4, 2, 6), col);
	glyph_box(s, lp_rect(c.x - 4, c.y - 2, 7, 7), 2, col);
}

static void	glyph_close(t_surface *s, t_point c, t_color col)
{
	t_lline	l;

	l = (t_lline){lp_pt(c.x * 16 - 56, c.y * 16 - 56), lp_pt(c.x * 16 + 56,
			c.y * 16 + 56), 28, true};
	lp_line(s, &l, col);
	l = (t_lline){lp_pt(c.x * 16 + 56, c.y * 16 - 56), lp_pt(c.x * 16 - 56,
			c.y * 16 + 56), 28, true};
	lp_line(s, &l, col);
}

void	lp_capglyph(t_surface *s, const t_lcb *cb, t_color col)
{
	t_point	c;

	c = lp_pt(cb->r.x + cb->r.w / 2, cb->r.y + cb->r.h / 2);
	if (cb->kind == HT_MINBUTTON)
		lp_fill(s, lp_rect(c.x - 5, c.y, 10, 2), col);
	else if (cb->kind == HT_MAXBUTTON && !cb->maximized)
		glyph_box(s, lp_rect(c.x - 4, c.y - 4, 9, 9), 2, col);
	else if (cb->kind == HT_MAXBUTTON)
		glyph_restore(s, c, col);
	else
		glyph_close(s, c, col);
}
