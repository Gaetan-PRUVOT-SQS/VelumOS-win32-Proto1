#include "luna_ctrl.h"

void	lp_sthumb(t_surface *s, const t_rect *r, t_lunastate st, bool vert)
{
	t_lrfill	f;
	t_color		c;
	t_rect		in;

	if (r->w <= 0 || r->h <= 0)
		return ;
	c = lp_tone_alpha(lp_pal()->muted, 0x80);
	if (st == LS_HOT)
		c = lp_tone_alpha(lp_pal()->muted, 0xb3);
	if (st == LS_PRESSED)
		c = lp_pal()->accent;
	in = lp_rect(r->x + 2, r->y, lp_max(r->w - 4, 1), r->h);
	if (!vert)
		in = lp_rect(r->x, r->y + 2, r->w, lp_max(r->h - 4, 1));
	f = lp_rmake(in, LP_RMAX, c, c);
	lp_rfill(s, &f);
}
