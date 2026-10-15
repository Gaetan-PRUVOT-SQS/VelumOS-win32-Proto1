#include "velum/libk.h"
#include "logon.h"
#include "logon_glass.h"
#include "logon_paint.h"

static void	card_backdrop(t_logon *lg)
{
	t_surface	*s;
	t_rect		saved;

	s = &lg->ui.win.surface;
	saved = s->clip;
	gfx_set_clip(s, rect_intersect(saved, lg->lay.card));
	luna_logon_bg(s, rect_make(0, 0, s->w, s->h));
	gfx_set_clip(s, saved);
}

void	logon_paint_card(t_logon *lg)
{
	t_surface	*s;
	t_rect		a;

	s = &lg->ui.win.surface;
	card_backdrop(lg);
	logon_glass_open(lg->lay.card);
	luna_glass(s, lg->lay.card, LUNA_GLASS_CARD);
	if (lg->lay.count != 1 || lg->flow.mode == LM_SHUTDOWN)
		return ;
	a = lg->lay.avatar;
	a.x += lg->lay.card.x;
	a.y += lg->lay.card.y;
	luna_glass(s, a, LUNA_GLASS_CHIP);
}

void	logon_underlay(void *user, t_rect dirty)
{
	t_logon	*lg;

	lg = user;
	if (rect_empty(rect_intersect(dirty, lg->lay.card)))
		return ;
	logon_paint_card(lg);
	ctl_invalidate(&lg->ui.root, lg->ui.root.root);
}
