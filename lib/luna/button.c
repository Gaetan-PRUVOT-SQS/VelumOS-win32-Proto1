#include "luna_ctrl.h"
#include "velum/theme.h"

static t_color	btn_shade(t_color c, t_lunastate st)
{
	if (st == LS_HOT)
		return (lp_mix(c, LC_WHITE, 36));
	if (st == LS_PRESSED)
		return (lp_mix(c, LC_BLACK, 56));
	return (c);
}

static void	btn_primary(t_surface *s, const t_lunabtn *b, int32_t rad)
{
	t_lrfill	f;

	f = lp_rmake(b->r, rad, btn_shade(VT_ACCENT_HI, b->state),
			btn_shade(VT_ACCENT, b->state));
	lp_rfill(s, &f);
	f.top = LCT_RIM_PRIMARY;
	f.bottom = LCT_RIM_PRIMARY;
	lp_rtop(s, &f);
}

static void	btn_body(t_surface *s, const t_lunabtn *b, int32_t rad)
{
	t_lrfill	f;
	t_color		fill;

	fill = lp_pal()->control;
	if (b->state == LS_DISABLED)
		fill = lp_tone_alpha(fill, (fill >> 24) / 2);
	f = lp_rmake(lp_inset(b->r, 1), rad - 1, fill, fill);
	lp_rfill(s, &f);
	if (b->state != LS_HOT && b->state != LS_PRESSED)
		return ;
	f.top = lp_tone_hover();
	if (b->state == LS_PRESSED)
		f.top = lp_tone_press();
	f.bottom = f.top;
	lp_rfill(s, &f);
}

static void	btn_rims(t_surface *s, const t_lunabtn *b, int32_t rad)
{
	t_cpal		pal;
	t_lrfill	f;
	t_color		edge;

	pal = lp_pal();
	edge = lp_tone_alpha(pal->muted, 0x59);
	f = lp_rmake(b->r, rad, edge, edge);
	lp_rring(s, &f, 1);
	f = lp_rmake(lp_inset(b->r, 1), rad - 1, pal->rim_top, pal->rim_top);
	lp_rtop(s, &f);
}

void	luna_button(t_surface *s, const t_lunabtn *src)
{
	t_lunabtn	b;
	int32_t		rad;

	if (!lp_ok(s) || src == NULL)
		return ;
	b = *src;
	b.r = lp_clean(src->r);
	if (b.r.w <= 0 || b.r.h <= 0)
		return ;
	rad = lp_min(LCT_RAD_CTRL, b.r.h / 2);
	if (lp_btn_primary(&b))
		btn_primary(s, &b, rad);
	else
		btn_body(s, &b, rad);
	if (!lp_btn_primary(&b) && b.state != LS_DISABLED)
		btn_rims(s, &b, rad);
	if ((b.focus || b.state == LS_FOCUSED) && b.state != LS_DISABLED)
		lp_rfocus(s, b.r, rad, lp_pal()->accent);
}
