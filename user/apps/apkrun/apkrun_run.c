#include "velum/abi/abi_syscall.h"
#include "velum/libk.h"
#include "apkrun.h"

void	apkrun_on_command(void *c, uint32_t code, void *user)
{
	const t_ctl	*ctl;
	t_apkrun	*app;

	ctl = c;
	app = user;
	if (!app || !ctl || code != CN_CLICKED || ctl->type != CT_BUTTON)
		return ;
	if (ctl->id > APKRUN_ID_BASE)
		app->click = ctl->id - APKRUN_ID_BASE;
}

static int	drain(t_apkrun *app)
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

static void	pump(t_apkrun *app)
{
	uint32_t	view;

	view = app->click;
	app->click = 0;
	if (view && app->started && !app->dead)
		apkrun_click(app, view);
	if (app->d && app->d->finished)
		app->quit = 1;
	if (app->laid.w != app->ui.area.w || app->laid.h != app->ui.area.h)
		app->redraw = 1;
	if (app->redraw || (app->d && app->d->dirty))
		apkrun_rebuild(app);
}

void	apkrun_run(t_apkrun *app)
{
	apkrun_rebuild(app);
	while (!app->quit)
	{
		if (drain(app) < 0)
			return ;
		pump(app);
		if (app->quit)
			return ;
		if (ui_wait(0, TIMEOUT_INF) != UI_WAIT_MSG)
			return ;
	}
}
