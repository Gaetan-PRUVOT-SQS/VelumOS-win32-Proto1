#include "velum/err.h"
#include "velum/libk.h"
#include "velum/abi/abi_syscall.h"
#include "velum/luna.h"
#include "../common/platform.h"
#include "notif_app.h"

static t_rect	column(const t_wmhello *info)
{
	int32_t	h;

	h = (int32_t)info->screen_h - NA_MARGIN_TOP - NA_MARGIN_BOTTOM;
	return (rect_make((int32_t)info->screen_w - NA_W - NA_MARGIN_X,
			NA_MARGIN_TOP, NA_W, h));
}

int	notifapp_open(t_notifapp *app, int open_now)
{
	t_wmhello	info;
	t_wmcreate	rq;
	int			r;

	memset(app, 0, sizeof(*app));
	notif_store_init(&app->store);
	r = notifsrv_start(app);
	if (r < 0)
		return (r);
	r = ui_connect(&info);
	if (r < 0)
		return (r);
	ui_request(&rq, column(&info), WS_CAPTION | WS_TOOLWINDOW, NA_TITLE);
	rq.state = open_now ? WSTATE_NORMAL : WSTATE_HIDDEN;
	r = uiwin_open(&app->ui, &rq, notifui_command);
	if (r < 0)
		return (r);
	app->ui.root.user = app;
	app->visible = open_now;
	if (os_timer_open(&app->timer) < 0)
		return (E_NOMEM);
	os_timer_after(app->timer, NA_TICK_NS, NA_TICK_NS);
	return (notifui_rebuild(app));
}

static int	drain(t_notifapp *app)
{
	t_uimsg	m;
	int		n;

	n = ui_next(&m);
	while (n > 0)
	{
		if (uiwin_event(&app->ui, &m) == UIE_CLOSE)
			notifui_show(app, 0);
		n = ui_next(&m);
	}
	return (n);
}

static void	tick(t_notifapp *app)
{
	notifsrv_poll(app);
	app->ticks++;
	if (!app->visible)
		return ;
	if (app->dirty || app->ticks % NA_AGE_TICKS == 0)
		notifui_rebuild(app);
}

void	notifapp_run(t_notifapp *app)
{
	int	w;

	while (!app->quit)
	{
		if (drain(app) < 0)
			return ;
		w = ui_wait(app->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
			tick(app);
		else if (w != UI_WAIT_MSG)
			return ;
	}
}

void	notifapp_close(t_notifapp *app)
{
	notifsrv_stop(app);
	if (app->timer != 0)
		os_close(app->timer);
	uiwin_close(&app->ui);
}
