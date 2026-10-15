#include "velum/libk.h"
#include "velum/luna_dock.h"
#include "../common/layout.h"
#include "shell.h"

void	sh_layout(t_shell *sh)
{
	t_tbmetrics	m;
	t_rect		area;

	m = (t_tbmetrics){(int32_t)sh->info.screen_w, sh->lm.taskbar_h,
		VT_DOCK_MARGIN, VT_DOCK_PAD_X, VT_DOCK_GAP, sh->lm.start_w,
		LD_TRAY_W, VT_TRAY_H, VT_TASK_H};
	tb_regions(&m, &sh->reg);
	area = lay_rect(0, 0, m.w, (int32_t)sh->info.screen_h - sh->lm.taskbar_h);
	sh->placed = (uint32_t)desk_grid(area, DESK_ICONS, sh->cells);
	sh_menu_geo(sh);
}

static int	make_bar(t_shell *sh)
{
	t_wmcreate	rq;
	int32_t		w;
	int32_t		h;

	w = (int32_t)sh->info.screen_w;
	h = (int32_t)sh->info.screen_h;
	ui_request(&rq, lay_rect(0, h - sh->lm.taskbar_h, w, sh->lm.taskbar_h),
		WS_APPBAR, "Barre des tâches");
	return (wmc_create(&sh->bar, &rq));
}

static int	make_menu(t_shell *sh)
{
	t_wmcreate	rq;
	t_rect		at;

	at = luna_launcher_place((int32_t)sh->info.screen_h, sh->lm.taskbar_h);
	ui_request(&rq, at, WS_POPUP | WS_TOPMOST | WS_NOACTIVATE,
		"Menu Démarrer");
	rq.state = WSTATE_HIDDEN;
	return (wmc_create(&sh->menu, &rq));
}

int	sh_make_windows(t_shell *sh)
{
	t_wmcreate	rq;
	int			r;

	sh_layout(sh);
	sh_glass_make(sh);
	ui_request(&rq, lay_rect(0, 0, (int32_t)sh->info.screen_w,
			(int32_t)sh->info.screen_h), WS_DESKTOP, "Bureau");
	r = wmc_create(&sh->desk, &rq);
	if (r == 0)
		r = make_bar(sh);
	if (r == 0)
		r = make_menu(sh);
	return (r);
}
