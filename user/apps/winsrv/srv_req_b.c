#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

int	ws_req_title(t_wsrv *s, int ci, const void *msg)
{
	const t_wmtitle	*rq;
	int				slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	memset(s->t.w[slot].title, 0, sizeof(s->t.w[slot].title));
	strlcpy(s->t.w[slot].title, rq->title, sizeof(s->t.w[slot].title));
	ws_mark_win(s, slot);
	ws_notify(s, slot, WMS_WIN_UPD);
	return (0);
}

int	ws_req_rect(t_wsrv *s, int ci, const void *msg)
{
	const t_wmrect	*rq;
	t_rect			r;
	int				slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	r = wg_normalize(&s->t, s->t.w[slot].style, rq->rect);
	if (s->t.w[slot].state != WSTATE_NORMAL)
	{
		s->t.w[slot].normal = r;
		return (0);
	}
	if (s->drag.slot == slot)
		s->drag.slot = -1;
	ws_apply_rect(s, slot, r, true);
	if (s->t.w[slot].style & WS_APPBAR)
		ws_workarea(s);
	return (0);
}

int	ws_req_state(t_wsrv *s, int ci, const void *msg)
{
	const t_wmarg	*rq;
	int				slot;
	uint32_t		old;

	rq = msg;
	slot = ws_target(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	old = s->t.w[slot].state;
	ws_set_state(s, slot, rq->value);
	if (ci == s->shell && old == WSTATE_MIN
		&& wf_activatable(&s->t.w[slot]))
		ws_activate(s, slot);
	return (0);
}

int	ws_req_cursor(t_wsrv *s, int ci, const void *msg)
{
	const t_wmarg	*rq;
	int				slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	s->t.w[slot].cursor = rq->value;
	if (s->m.hover == slot)
		ws_hover(s);
	return (0);
}

int	ws_req_icon(t_wsrv *s, int ci, const void *msg)
{
	const t_wmarg	*rq;
	int				slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	s->t.w[slot].icon = rq->value;
	ws_mark_win(s, slot);
	ws_notify(s, slot, WMS_WIN_UPD);
	return (0);
}
