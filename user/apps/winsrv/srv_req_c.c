#include "velum/err.h"
#include "ws_srv.h"

int	ws_req_present(t_wsrv *s, int ci, const void *msg)
{
	const t_wmpresent	*rq;
	t_rect				cl;
	uint32_t			i;
	int					slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	if (!wf_visible(&s->t.w[slot]))
		return (0);
	cl = wh_client(&s->t, slot);
	if (rq->nrects == 0)
		ws_mark(s, cl);
	i = 0;
	while (i < rq->nrects)
	{
		ws_mark(s, rect_intersect(rect_make(cl.x + rq->rects[i].x,
					cl.y + rq->rects[i].y, rq->rects[i].w, rq->rects[i].h),
				cl));
		i++;
	}
	return (0);
}

int	ws_req_activate(t_wsrv *s, int ci, const void *msg)
{
	const t_wmhdr	*rq;
	int				slot;
	int				act;

	rq = msg;
	slot = ws_target(s, ci, rq->window);
	if (slot < 0)
		return (slot);
	if (s->t.w[slot].state == WSTATE_HIDDEN
		|| (s->t.w[slot].style & (WS_NOACTIVATE | WS_APPBAR)))
		return (E_INVAL);
	act = s->t.active;
	if (ci != s->shell && act >= 0 && s->t.w[act].owner != ci)
		return (E_PERM);
	ws_activate(s, slot);
	return (0);
}

int	ws_req_workarea(t_wsrv *s, int ci, const void *msg)
{
	t_rect	r;

	if (ws_claim_shell(s, ci) < 0)
		return (E_PERM);
	r = rect_intersect(((const t_wmrect *)msg)->rect, s->t.screen);
	if (rect_empty(r))
		return (E_INVAL);
	s->work_set = r;
	ws_workarea(s);
	return (0);
}

int	ws_req_subscribe(t_wsrv *s, int ci, const void *msg)
{
	if (ws_claim_shell(s, ci) < 0)
		return (E_PERM);
	if (((const t_wmarg *)msg)->value == 0)
	{
		s->c[ci].flags &= ~(uint32_t)WCF_SUB;
		return (0);
	}
	s->c[ci].flags |= WCF_SUB;
	ws_notify_all(s);
	return (0);
}

int	ws_req_capture(t_wsrv *s, int ci, const void *msg)
{
	const t_wmarg	*rq;
	int				slot;

	rq = msg;
	slot = ws_own(s, ci, rq->h.window);
	if (slot < 0)
		return (slot);
	if (rq->value == 0)
	{
		if (s->m.capture == slot)
		{
			s->m.capture = -1;
			s->m.explicit_cap = false;
		}
		return (0);
	}
	if (slot != s->t.active && ci != s->shell)
		return (E_PERM);
	if (!wf_visible(&s->t.w[slot]))
		return (E_INVAL);
	ws_drag_end(s);
	s->m.capture = slot;
	s->m.explicit_cap = true;
	return (0);
}
