#include "velum/libk.h"
#include "ws_srv.h"

void	ws_release_input(t_wsrv *s, int slot)
{
	if (s->m.capture == slot)
	{
		s->m.capture = -1;
		s->m.explicit_cap = false;
	}
	if (s->m.hover == slot)
		s->m.hover = -1;
	if (s->m.hot_slot == slot)
		s->m.hot_slot = -1;
	if (s->m.press_slot == slot)
		s->m.press_slot = -1;
	if (s->m.click_slot == slot)
		s->m.click_slot = -1;
	if (s->drag.slot == slot)
		s->drag.slot = -1;
}

void	ws_win_destroy(t_wsrv *s, int slot)
{
	bool	was_active;
	bool	appbar;

	ws_release_input(s, slot);
	ws_mark_win(s, slot);
	ws_notify(s, slot, WMS_WIN_DEL);
	ws_section_release(s, slot);
	was_active = (s->t.active == slot);
	appbar = (s->t.w[slot].style & WS_APPBAR) != 0;
	wt_free(&s->t, slot);
	if (appbar)
		ws_workarea(s);
	if (was_active)
		ws_activate(s, wf_next(&s->t, -1));
}

static void	send_activation(t_wsrv *s, int slot, uint32_t on)
{
	t_wmarg	m;

	if (slot < 0)
		return ;
	ws_mark_win(s, slot);
	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMS_ACTIVATE, sizeof(m), s->t.w[slot].id);
	m.value = on;
	ws_post(s, s->t.w[slot].owner, &m, 0);
	ws_notify(s, slot, WMS_WIN_UPD);
}

void	ws_activate(t_wsrv *s, int slot)
{
	int	old;

	if (slot >= 0 && s->t.w[slot].state == WSTATE_MIN)
		ws_state_apply(s, slot, s->t.w[slot].before_min);
	if (slot >= 0 && !wf_activatable(&s->t.w[slot]))
		return ;
	if (slot >= 0 && wz_raise(&s->t, slot))
		ws_mark_win(s, slot);
	old = s->t.active;
	if (old == slot)
		return ;
	s->t.active = slot;
	send_activation(s, old, 0);
	send_activation(s, slot, 1);
}

void	ws_apply_rect(t_wsrv *s, int slot, t_rect r, bool final)
{
	t_wwin	*w;
	t_rect	cl;

	w = &s->t.w[slot];
	if (memcmp(&r, &w->rect, sizeof(r)) != 0)
	{
		ws_mark_win(s, slot);
		w->rect = r;
		ws_mark_win(s, slot);
	}
	if (!final)
		return ;
	cl = wh_client(&s->t, slot);
	if (w->content.px != NULL && w->content.w == wg_clamp(cl.w, 1, WS_DIM_MAX)
		&& w->content.h == wg_clamp(cl.h, 1, WS_DIM_MAX))
		return ;
	if (ws_section_attach(s, slot) != 0)
		return ;
	ws_mark_win(s, slot);
	ws_send_section(s, slot, WMS_RESIZED, 0);
}
