#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "../common/timefmt.h"
#include "hello.h"

int	hello_open(t_hello *app)
{
	t_wmhello	info;
	t_wmcreate	rq;
	int			r;

	memset(app, 0, sizeof(*app));
	r = ui_connect(&info);
	if (r < 0)
		return (r);
	ui_request(&rq, rect_make(HELLO_X, HELLO_Y, HELLO_W, HELLO_H_PX),
		WS_DEFAULT, HELLO_TITLE);
	r = uiwin_open(&app->ui, &rq, hello_on_command);
	if (r < 0)
		return (r);
	app->ui.root.user = app;
	if (hello_build(app) < 0 || os_timer_open(&app->timer) < 0)
		return (E_NOMEM);
	os_timer_after(app->timer, NS_PER_SEC, NS_PER_SEC);
	hello_tick(app);
	return (0);
}

static int	drain(t_hello *app)
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

void	hello_run(t_hello *app)
{
	int	w;

	while (!app->quit)
	{
		if (drain(app) < 0)
			return ;
		if (app->quit)
			return ;
		w = ui_wait(app->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
			hello_tick(app);
		else if (w != UI_WAIT_MSG)
			return ;
	}
}
