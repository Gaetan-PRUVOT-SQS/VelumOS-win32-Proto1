#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "reglages.h"

uint32_t	reg_hour(void)
{
	return ((uint32_t)((os_wall_ns() / REG_HOUR_NS) % 24));
}

void	reg_apply(t_reglages *app, int force)
{
	uint32_t	dark;
	uint32_t	reduced;

	dark = set_dark_at(&app->cfg, reg_hour()) ? 1 : 0;
	reduced = app->cfg.reduced;
	if (!force && dark == app->sent_dark && reduced == app->sent_reduced)
		return ;
	if (wmc_set_theme(&app->ui.win, dark, reduced) < 0)
		return ;
	app->sent_dark = dark;
	app->sent_reduced = reduced;
}

int	reg_open(t_reglages *app)
{
	t_wmhello	info;
	t_wmcreate	rq;
	int32_t		x;
	int			r;

	memset(app, 0, sizeof(*app));
	r = ui_connect(&info);
	if (r < 0)
		return (r);
	x = ((int32_t)info.screen_w - REG_W) / 2;
	ui_request(&rq, rect_make(x < 0 ? 0 : x, REG_Y, REG_W, REG_H),
		WS_DEFAULT, REG_TITLE);
	r = uiwin_open(&app->ui, &rq, reg_on_command);
	if (r < 0)
		return (r);
	app->ui.root.user = app;
	set_load(SET_PATH, &app->cfg);
	if (reg_build(app) < 0 || os_timer_open(&app->timer) < 0)
		return (E_NOMEM);
	os_timer_after(app->timer, REG_TICK_NS, REG_TICK_NS);
	reg_apply(app, 1);
	uiwin_flush(&app->ui);
	return (0);
}

static int	drain(t_reglages *app)
{
	t_uimsg	m;
	int		n;

	n = ui_next(&m);
	while (n > 0)
	{
		if (uiwin_event(&app->ui, &m) == UIE_CLOSE)
			app->quit = 1;
		n = ui_next(&m);
	}
	return (n);
}

void	reg_run(t_reglages *app)
{
	int	w;

	while (!app->quit)
	{
		if (drain(app) < 0 || app->quit)
			return ;
		w = ui_wait(app->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
			reg_apply(app, 0);
		else if (w != UI_WAIT_MSG)
			return ;
	}
}
