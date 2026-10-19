#include "velum/abi/abi_input.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "../common/platform.h"
#include "con_app.h"

static void	typed_char(t_conapp *a, const t_inpevent *ev)
{
	char		buf[4];
	uint32_t	n;
	int			alt;
	int			ctrl;

	alt = (ev->mods & INPM_ALT) != 0;
	ctrl = (ev->mods & INPM_CTRL) != 0;
	if (alt != ctrl || ev->code < 0x20 || (ev->code >= 0x7f
			&& ev->code < 0xa0))
		return ;
	n = utf8_put(ev->code, buf);
	line_insert(&a->con.line, buf, n);
}

static void	cancel_line(t_conapp *a)
{
	char	typed[CON_LINE_MAX + 4];

	strlcpy(typed, a->con.line.buf, sizeof(typed));
	strlcat(typed, "^C", sizeof(typed));
	con_echo(&a->con, typed);
	line_kill_all(&a->con.line);
}

static void	ctrl_key(t_conapp *a, uint32_t code)
{
	t_conline	*l;

	l = &a->con.line;
	if (code == 'L')
		scr_clear(&a->con.screen);
	else if (code == 'C')
		cancel_line(a);
	else if (code == 'U')
		line_kill_all(l);
	else if (code == 'K')
		line_kill_end(l);
	else if (code == 'A')
		line_home(l);
	else if (code == 'E')
		line_end(l);
}

static void	scroll_by(t_conapp *a, int dir)
{
	uint32_t	step;
	uint32_t	max;

	step = a->rows_vis / 2 + 1;
	max = scr_count(&a->con.screen);
	if (dir > 0)
		a->scroll = a->scroll + step > max ? max : a->scroll + step;
	else
		a->scroll = a->scroll > step ? a->scroll - step : 0;
}

static void	plain_key(t_conapp *a, uint32_t code)
{
	t_conline	*l;

	l = &a->con.line;
	if (code == VK_RETURN)
		con_submit(&a->con);
	else if (code == VK_BACK)
		line_backspace(l);
	else if (code == VK_DELETE)
		line_delete(l);
	else if (code == VK_LEFT)
		line_left(l);
	else if (code == VK_RIGHT)
		line_right(l);
	else if (code == VK_HOME)
		line_home(l);
	else if (code == VK_END)
		line_end(l);
	else if (code == VK_UP)
		line_hist_prev(l);
	else if (code == VK_DOWN)
		line_hist_next(l);
	else if (code == VK_PRIOR)
		scroll_by(a, 1);
	else if (code == VK_NEXT)
		scroll_by(a, -1);
}

void	con_on_key(t_conapp *a, const t_inpevent *ev)
{
	int	alt;
	int	ctrl;

	alt = (ev->mods & INPM_ALT) != 0;
	ctrl = (ev->mods & INPM_CTRL) != 0;
	if (ev->type == INP_CHAR)
		typed_char(a, ev);
	else if (ev->type == INP_KEY_DOWN && ctrl && !alt)
		ctrl_key(a, ev->code);
	else if (ev->type == INP_KEY_DOWN && !alt && !ctrl)
		plain_key(a, ev->code);
	else
		return ;
	if (ev->code != VK_PRIOR && ev->code != VK_NEXT)
		a->scroll = 0;
	a->blink = 1;
	if (a->con.quit)
		a->quit = 1;
}

void	con_layout(t_conapp *a)
{
	int32_t	w;
	int32_t	h;
	int32_t	cols;

	w = a->ui.win.surface.w;
	h = a->ui.win.surface.h;
	cols = (w - 2 * CON_PAD_X) / a->cell_w;
	a->rows_vis = 1;
	if (h > 2 * CON_PAD_Y + CON_STATUS_H + CON_LINE_H)
		a->rows_vis = (uint32_t)((h - 2 * CON_PAD_Y - CON_STATUS_H)
				/ CON_LINE_H);
	scr_set_cols(&a->con.screen, cols > 0 ? (uint32_t)cols : 8);
}

void	con_dispatch(t_conapp *a, const t_uimsg *m)
{
	const t_wmhdr	*h;
	const t_wmkey	*k;
	int				r;

	h = (const t_wmhdr *)m->words;
	if (!a->ui.ready || (size_t)m->len < sizeof(*h)
		|| h->window != a->ui.win.id)
		return ;
	if (h->type == WMS_KEY && (size_t)m->len >= sizeof(*k))
	{
		k = (const t_wmkey *)m->words;
		con_on_key(a, &k->ev);
		con_paint(a);
		return ;
	}
	r = uiwin_event(&a->ui, m);
	if (r == UIE_CLOSE)
		a->quit = 1;
	if (h->type == WMS_RESIZED)
	{
		con_layout(a);
		con_paint(a);
	}
}
