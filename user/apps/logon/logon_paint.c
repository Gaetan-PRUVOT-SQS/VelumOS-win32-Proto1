#include "velum/libk.h"
#include "../common/layout.h"
#include "../common/platform.h"
#include "logon.h"
#include "logon_glass.h"
#include "logon_paint.h"

void	logon_overlay(void *user, t_rect dirty)
{
	t_logon	*lg;

	(void)dirty;
	lg = user;
	if (lg->flow.mode == LM_SHUTDOWN)
		return ;
	logon_draw_avatars(lg);
}

static int	place(t_logon *lg)
{
	int32_t	w;
	int32_t	h;

	w = lg->ui.win.surface.w;
	h = lg->ui.win.surface.h;
	if (logon_layout(w, h, lg->nshown, &lg->lay) < 0)
		return (-1);
	return (uiwin_area(&lg->ui, lg->lay.card));
}

static void	paint_backdrop(t_logon *lg)
{
	t_surface	*s;

	s = &lg->ui.win.surface;
	luna_logon_bg(s, rect_make(0, 0, s->w, s->h));
	logon_paint_card(lg);
	logon_draw_chips(lg);
	logon_draw_head(lg, os_wall_ns());
}

int	logon_window(t_logon *lg)
{
	t_wmcreate	rq;
	int			r;

	ui_request(&rq, rect_make(0, 0, (int32_t)lg->info.screen_w,
			(int32_t)lg->info.screen_h), WS_FULLSCREEN, "Ouverture de session");
	r = uiwin_open(&lg->ui, &rq, logon_on_command);
	if (r < 0)
		return (r);
	lg->ui.root.user = lg;
	lg->ui.underlay = logon_underlay;
	lg->ui.underlay_user = lg;
	lg->ui.overlay = logon_overlay;
	lg->ui.overlay_user = lg;
	ctl_set_flag(&lg->ui.root, lg->ui.root.root, CTL_NOBACK, true);
	if (place(lg) < 0)
		return (-1);
	paint_backdrop(lg);
	if (logon_build(lg) < 0)
		return (-1);
	wmc_present(&lg->ui.win, NULL, 0);
	return (0);
}
