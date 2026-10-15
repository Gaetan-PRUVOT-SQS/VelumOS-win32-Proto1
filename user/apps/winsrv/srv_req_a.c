#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

int	ws_req_hello(t_wsrv *s, int ci, const void *msg)
{
	t_wmhello	rq;
	t_wmhello	ok;

	memcpy(&rq, msg, sizeof(rq));
	s->c[ci].flags |= WCF_HELLO;
	memset(&ok, 0, sizeof(ok));
	ws_hdr(&ok.h, WMS_HELLO_OK, sizeof(ok), 0);
	ok.h.seq = rq.h.seq;
	ok.version = WM_VERSION;
	ok.screen_w = (uint32_t)s->t.screen.w;
	ok.screen_h = (uint32_t)s->t.screen.h;
	ok.workarea = s->t.work;
	ws_post(s, ci, &ok, 0);
	return (0);
}

static void	init_window(t_wsrv *s, int slot, const t_wmcreate *rq)
{
	t_wwin	*w;

	w = &s->t.w[slot];
	w->style = rq->style;
	w->state = rq->state;
	w->icon = ICON_NONE;
	w->cursor = CUR_ARROW;
	w->before_min = WSTATE_NORMAL;
	memset(w->title, 0, sizeof(w->title));
	strlcpy(w->title, rq->title, sizeof(w->title));
	w->rect = wg_normalize(&s->t, rq->style, rq->rect);
	w->normal = w->rect;
	if (w->state == WSTATE_MAX)
		w->rect = wg_maximized(&s->t);
}

static void	announce(t_wsrv *s, int slot, uint32_t seq)
{
	wz_insert(&s->t, slot);
	ws_send_section(s, slot, WMS_CREATED, seq);
	ws_mark_win(s, slot);
	ws_notify(s, slot, WMS_WIN_ADD);
	if (s->t.w[slot].style & WS_APPBAR)
		ws_workarea(s);
	if (wf_activatable(&s->t.w[slot]))
		ws_activate(s, slot);
}

int	ws_req_create(t_wsrv *s, int ci, const void *msg)
{
	const t_wmcreate	*rq;
	int					slot;
	int					r;

	rq = msg;
	if ((rq->style & (WS_DESKTOP | WS_APPBAR)) && ws_claim_shell(s, ci) < 0)
		return (E_PERM);
	if (wt_count_owner(&s->t, ci) >= WM_WIN_PER_CLIENT)
		return (E_NOSPC);
	slot = wt_alloc(&s->t, ci);
	if (slot < 0)
		return (slot);
	init_window(s, slot, rq);
	r = ws_section_attach(s, slot);
	if (r < 0)
	{
		wt_free(&s->t, slot);
		return (r);
	}
	announce(s, slot, rq->h.seq);
	return (0);
}

int	ws_req_destroy(t_wsrv *s, int ci, const void *msg)
{
	int	slot;

	slot = ws_own(s, ci, ((const t_wmhdr *)msg)->window);
	if (slot < 0)
		return (slot);
	ws_win_destroy(s, slot);
	return (0);
}
