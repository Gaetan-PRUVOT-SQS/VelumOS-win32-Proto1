#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "velum/font.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "con_app.h"

static t_conapp	g_app;

static int32_t	clamp_extent(int32_t want, int32_t avail)
{
	if (avail > 0 && want > avail)
		return (avail);
	return (want);
}

int	con_open(t_conapp *a)
{
	t_wmhello	info;
	t_wmcreate	rq;
	int			r;

	memset(a, 0, sizeof(*a));
	r = ui_connect(&info);
	if (r < 0)
		return (r);
	ui_request(&rq, rect_make(CON_WIN_X, CON_WIN_Y,
			clamp_extent(CON_WIN_W, info.workarea.w - CON_WIN_X),
			clamp_extent(CON_WIN_H, info.workarea.h - CON_WIN_Y)),
		WS_DEFAULT, CON_TITLE);
	r = uiwin_open(&a->ui, &rq, NULL);
	if (r < 0)
		return (r);
	a->cell_w = font_glyph(font_get(FONT_JBM_14), 'M')->advance;
	con_init(&a->con, &g_conops_system, a, 80);
	if (os_timer_open(&a->timer) < 0)
		return (E_NOMEM);
	os_timer_after(a->timer, CON_BLINK_NS, CON_BLINK_NS);
	a->blink = 1;
	con_layout(a);
	con_say(&a->con, "VelumOS · console", CC_DIM);
	con_say(&a->con, "", CC_TEXT);
	return (0);
}

static int	drain(t_conapp *a)
{
	t_uimsg	m;
	int		n;

	n = ui_next(&m);
	while (n > 0)
	{
		con_dispatch(a, &m);
		n = ui_next(&m);
	}
	return (n);
}

void	con_loop(t_conapp *a)
{
	int	w;

	con_paint(a);
	while (!a->quit)
	{
		if (drain(a) < 0 || a->quit)
			return ;
		w = ui_wait(a->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
		{
			a->blink = !a->blink;
			con_jobs_reap(&a->jobs);
			con_paint(a);
		}
		else if (w != UI_WAIT_MSG)
			return ;
	}
}

int	main(void)
{
	if (con_open(&g_app) < 0)
	{
		os_log("console: connexion au serveur de fenêtres impossible");
		return (1);
	}
	os_log("console: fenêtre prête");
	con_loop(&g_app);
	uiwin_close(&g_app.ui);
	return (0);
}
