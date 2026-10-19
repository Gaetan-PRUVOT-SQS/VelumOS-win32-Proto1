#include "luna_ctrl.h"

void	luna_progress(t_surface *s, t_rect r, uint32_t pct)
{
	t_lrfill	f;
	int32_t		w;

	r = lp_clean(r);
	if (!lp_ok(s) || r.w < 2 || r.h < 2)
		return ;
	f = lp_rmake(r, r.h / 2, lp_tone_track(), lp_tone_track());
	lp_rfill(s, &f);
	if (pct > 100)
		pct = 100;
	w = (int32_t)((int64_t)r.w * pct / 100);
	if (w <= 0)
		return ;
	f = lp_rmake(lp_rect(r.x, r.y, w, r.h), r.h / 2, lp_pal()->accent,
			lp_pal()->accent);
	lp_rfill(s, &f);
}
