#include "velum/libk.h"
#include "ws_srv.h"

static void	state_message(t_wsrv *s, int slot)
{
	t_wmarg	m;

	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMS_STATE, sizeof(m), s->t.w[slot].id);
	m.value = s->t.w[slot].state;
	ws_post(s, s->t.w[slot].owner, &m, 0);
	ws_notify(s, slot, WMS_WIN_UPD);
}

void	ws_state_apply(t_wsrv *s, int slot, uint32_t state)
{
	t_wwin		*w;
	uint32_t	old;

	w = &s->t.w[slot];
	old = w->state;
	if (old == state || state > WSTATE_HIDDEN)
		return ;
	ws_mark_win(s, slot);
	if (old == WSTATE_NORMAL)
		w->normal = w->rect;
	if (state == WSTATE_MIN || state == WSTATE_HIDDEN)
		ws_release_input(s, slot);
	if (state == WSTATE_MIN && old == WSTATE_MAX)
		w->before_min = WSTATE_MAX;
	else if (state == WSTATE_MIN)
		w->before_min = WSTATE_NORMAL;
	w->state = state;
	if (state == WSTATE_MAX)
		ws_apply_rect(s, slot, wg_maximized(&s->t), true);
	if (state == WSTATE_NORMAL)
		ws_apply_rect(s, slot, w->normal, true);
	ws_mark_win(s, slot);
	state_message(s, slot);
	if (w->style & WS_APPBAR)
		ws_workarea(s);
}

void	ws_set_state(t_wsrv *s, int slot, uint32_t state)
{
	bool	was_active;

	was_active = (s->t.active == slot);
	ws_state_apply(s, slot, state);
	if (was_active && !wf_activatable(&s->t.w[slot]))
		ws_activate(s, wf_next(&s->t, slot));
}

static t_rect	appbar_area(const t_wsrv *s)
{
	t_rect	r;
	t_rect	a;
	int		i;

	r = s->t.screen;
	i = 0;
	while (i < WS_WIN_MAX)
	{
		a = s->t.w[i].rect;
		if ((s->t.w[i].style & WS_APPBAR) && wf_visible(&s->t.w[i])
			&& a.x <= r.x && a.x + a.w >= r.x + r.w)
		{
			if (a.y > r.y + r.h / 2 && a.y < r.y + r.h)
				r.h = a.y - r.y;
			else if (a.y <= r.y && a.y + a.h > r.y && a.y + a.h < r.y + r.h)
			{
				r.h -= a.y + a.h - r.y;
				r.y = a.y + a.h;
			}
		}
		i++;
	}
	return (r);
}

void	ws_workarea(t_wsrv *s)
{
	t_rect	r;
	int		i;

	r = appbar_area(s);
	if (!rect_empty(s->work_set))
		r = s->work_set;
	if (memcmp(&r, &s->t.work, sizeof(r)) == 0)
		return ;
	s->t.work = r;
	i = 0;
	while (i < WS_WIN_MAX)
	{
		if (s->t.w[i].id != 0 && s->t.w[i].state == WSTATE_MAX)
			ws_apply_rect(s, i, r, true);
		i++;
	}
}
