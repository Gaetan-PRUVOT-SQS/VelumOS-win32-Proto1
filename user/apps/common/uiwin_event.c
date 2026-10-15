#include "velum/abi/abi_input.h"
#include "velum/libk.h"
#include "uiwin.h"

void	uiwin_resized(t_uiwin *u)
{
	if (!u->ready)
		return ;
	uiwin_area(u, rect_make(0, 0, u->win.surface.w, u->win.surface.h));
	uiwin_flush(u);
}

static int	on_key(t_uiwin *u, const t_uimsg *m)
{
	const t_wmkey	*k;

	if ((size_t)m->len < sizeof(*k))
		return (UIE_NONE);
	k = (const t_wmkey *)m->words;
	ctl_key(&u->root, &k->ev);
	uiwin_flush(u);
	return (UIE_USED);
}

static int	on_mouse(t_uiwin *u, const t_uimsg *m)
{
	t_wmmouse	ev;

	if ((size_t)m->len < sizeof(ev))
		return (UIE_NONE);
	memcpy(&ev, m->words, sizeof(ev));
	ev.x -= u->area.x;
	ev.y -= u->area.y;
	ctl_mouse(&u->root, &ev);
	uiwin_flush(u);
	return (UIE_USED);
}

int	uiwin_event(t_uiwin *u, const t_uimsg *m)
{
	const t_wmhdr	*h;

	h = (const t_wmhdr *)m->words;
	if (!u->ready || (size_t)m->len < sizeof(*h) || h->window != u->win.id)
		return (UIE_NONE);
	if (h->type == WMS_KEY)
		return (on_key(u, m));
	if (h->type == WMS_MOUSE)
		return (on_mouse(u, m));
	if (h->type == WMS_CLOSE_REQ)
		return (UIE_CLOSE);
	if (h->type == WMS_PAINT)
		wmc_present(&u->win, NULL, 0);
	if (h->type == WMS_RESIZED)
		uiwin_resized(u);
	return (UIE_USED);
}
