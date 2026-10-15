#include "luna_ctrl.h"

static void	sc_button(t_surface *s, t_rect r, t_lunastate st, int32_t dir)
{
	t_lrfill	f;
	t_color		arrow;

	arrow = lp_pal()->muted;
	if (st == LS_DISABLED)
		arrow = lp_pal()->dim;
	if (st == LS_HOT || st == LS_PRESSED)
	{
		f = lp_rmake(r, LP_RMAX, lp_tone_hover(), lp_tone_hover());
		if (st == LS_PRESSED)
		{
			f.top = lp_tone_press();
			f.bottom = f.top;
		}
		lp_rfill(s, &f);
	}
	lp_arrow(s, r, dir, arrow);
}

void	luna_scrollbar_ex(t_surface *s, const t_lunascroll *src)
{
	t_lunascroll	sb;
	t_scrollparts	p;
	t_lrfill		f;
	bool			vert;

	if (!lp_ok(s) || src == NULL)
		return ;
	sb = *src;
	sb.r = lp_clean(src->r);
	if (sb.r.w <= 0 || sb.r.h <= 0)
		return ;
	luna_scroll_layout(&sb, &p);
	vert = sb.r.h >= sb.r.w;
	f = lp_rmake(sb.r, LP_RMAX, lp_tone_track(), lp_tone_track());
	lp_rfill(s, &f);
	sc_button(s, p.up, sb.up, 2 - 2 * vert);
	sc_button(s, p.down, sb.down, 3 - 2 * vert);
	lp_sthumb(s, &p.thumb, sb.thumb, vert);
}

void	luna_scrollbar(t_surface *s, t_rect r, t_lunastate st)
{
	t_lunascroll	sb;

	sb.r = r;
	sb.total = 2;
	sb.page = 1;
	sb.pos = 0;
	sb.up = LS_NORMAL;
	sb.down = LS_NORMAL;
	sb.thumb = st;
	if (st == LS_DISABLED)
	{
		sb.total = 0;
		sb.up = LS_DISABLED;
		sb.down = LS_DISABLED;
	}
	luna_scrollbar_ex(s, &sb);
}
