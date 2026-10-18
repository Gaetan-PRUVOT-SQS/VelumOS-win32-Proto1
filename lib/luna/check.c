#include "luna_ctrl.h"
#include "velum/theme.h"

static const t_lop	g_tick[] = {{LOP_LINE, 1, {7, 15, 11, 20, 5}, LC_WHITE,
	LC_WHITE}, {LOP_LINE, 1, {11, 20, 20, 7, 5}, LC_WHITE, LC_WHITE}};

void	lp_choice_colors(const t_lchoice *c, t_lrfill *f, t_color *ring)
{
	t_cpal	pal;

	pal = lp_pal();
	f->top = pal->input;
	*ring = lp_tone_alpha(pal->muted, 0xa6);
	if (c->st == LS_HOT || c->on)
		*ring = pal->accent;
	if (c->st == LS_PRESSED)
		f->top = lp_tone_press();
	if (c->st == LS_DISABLED)
	{
		f->top = lp_tone_alpha(f->top, (f->top >> 24) / 2);
		*ring = lp_tone_alpha(pal->dim, 0xa6);
	}
	f->bottom = f->top;
	if (c->on && c->solid && c->st != LS_DISABLED)
	{
		f->top = VT_ACCENT_HI;
		f->bottom = VT_ACCENT;
	}
}

void	lp_choice(t_surface *s, const t_lchoice *c)
{
	t_lrfill	f;
	t_lrfill	edge;
	t_color		ring;

	lp_choice_colors(c, &f, &ring);
	edge = lp_rmake(c->r, c->rad, ring, ring);
	f.r = lp_inset(edge.r, 1);
	f.rad = lp_max(edge.rad - 1, 0);
	lp_rfill(s, &f);
	lp_rring(s, &edge, 1);
}

void	luna_checkbox(t_surface *s, t_point p, t_lunastate st, bool on)
{
	t_lchoice	c;
	t_lscale	sc;

	if (!lp_ok(s) || !lp_pt_ok(p))
		return ;
	c.r = lp_rect(p.x, p.y, lm_detail()->check_size, lm_detail()->check_size);
	c.rad = 4;
	c.st = st;
	c.on = on;
	c.solid = true;
	lp_choice(s, &c);
	sc.ox = p.x;
	sc.oy = p.y;
	sc.num = 1;
	sc.den = 2;
	if (on)
		lp_ops(s, &sc, g_tick, 2);
}

void	luna_radio(t_surface *s, t_point p, t_lunastate st, bool on)
{
	t_lchoice	c;

	if (!lp_ok(s) || !lp_pt_ok(p))
		return ;
	c.r = lp_rect(p.x, p.y, lm_detail()->check_size, lm_detail()->check_size);
	c.rad = lm_detail()->check_size / 2;
	c.st = st;
	c.on = on;
	c.solid = false;
	lp_radio(s, &c);
}
