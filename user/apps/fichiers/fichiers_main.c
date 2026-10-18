#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "fichiers.h"

static int64_t	clock_ns(void)
{
	return ((int64_t)os_mono_ns());
}

int	ff_app_open(t_fichiers *app)
{
	t_wmhello	info;
	t_wmcreate	rq;
	int			r;

	memset(app, 0, sizeof(*app));
	ff_ops_vfs(&app->ops);
	r = ui_connect(&info);
	if (r < 0)
		return (r);
	ui_request(&rq, rect_make(FF_X, FF_Y, FF_W, FF_H_PX), WS_DEFAULT,
		FF_TITLE);
	r = uiwin_open(&app->ui, &rq, ff_app_command);
	if (r < 0)
		return (r);
	app->ui.root.user = app;
	ctl_set_clock(&app->ui.root, clock_ns);
	if (ff_app_build(app) < 0)
		return (E_NOMEM);
	ff_app_goto(app, "/");
	return (0);
}

static int	drain(t_fichiers *app)
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

void	ff_app_run(t_fichiers *app)
{
	while (!app->quit)
	{
		if (drain(app) < 0 || app->quit)
			return ;
		if (ui_wait(0, TIMEOUT_INF) != UI_WAIT_MSG)
			return ;
	}
}
