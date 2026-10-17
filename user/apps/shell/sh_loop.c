#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "../common/platform.h"
#include "shell.h"

static void	on_winlist(t_shell *sh, const t_uimsg *m)
{
	const t_wminfo	*w;

	if ((size_t)m->len < sizeof(*w))
		return ;
	w = (const t_wminfo *)m->words;
	if (tl_apply(&sh->tasks, w->h.type, w))
		sh_bar_relayout(sh);
}

static void	on_window_event(t_shell *sh, const t_uimsg *m)
{
	const t_wmhdr	*h;

	h = (const t_wmhdr *)m->words;
	if (h->type == WMS_KEY && (size_t)m->len >= sizeof(t_wmkey))
		sh_on_key(sh, (const t_wmkey *)m->words);
	else if (h->type == WMS_MOUSE && (size_t)m->len >= sizeof(t_wmmouse))
		sh_on_mouse(sh, (const t_wmmouse *)m->words);
	else if (h->type == WMS_PAINT && h->window == sh->bar.id)
		sh_bar_paint(sh);
	else if (h->type == WMS_PAINT && h->window == sh->desk.id)
		sh_desk_paint(sh);
	else if (h->type == WMS_PAINT && h->window == sh->menu.id)
		sh_menu_paint(sh);
}

void	sh_dispatch(t_shell *sh, const t_uimsg *m)
{
	const t_wmhdr	*h;

	h = (const t_wmhdr *)m->words;
	if (sh->dlg.ready && h->window == sh->dlg.win.id)
	{
		if (uiwin_event(&sh->dlg, m) == UIE_CLOSE)
			sh_dlg_close(sh);
		sh_dlg_finish(sh);
		return ;
	}
	if (h->type == WMS_WIN_ADD || h->type == WMS_WIN_UPD
		|| h->type == WMS_WIN_DEL)
		on_winlist(sh, m);
	else
		on_window_event(sh, m);
}

static int	drain(t_shell *sh)
{
	t_uimsg	m;
	int		n;

	n = ui_next(&m);
	while (n > 0 && !sh->quit)
	{
		sh_dispatch(sh, &m);
		n = ui_next(&m);
	}
	return (n);
}

void	sh_run(t_shell *sh)
{
	int	w;

	while (!sh->quit)
	{
		if (drain(sh) < 0 || sh->quit)
			return ;
		sh_theme_sync(sh);
		w = ui_wait(sh->timer, TIMEOUT_INF);
		if (w == UI_WAIT_TIMER)
			sh_clock_tick(sh);
		else if (w != UI_WAIT_MSG)
			return ;
		sh_reap(sh);
	}
}
