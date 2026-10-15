#include "velum/libk.h"
#include "ws_srv.h"

void	ws_send_mouse(t_wsrv *s, int slot, uint32_t type, uint32_t extra)
{
	t_wmmouse	m;
	t_rect		cl;
	uint32_t	flags;

	cl = wh_client(&s->t, slot);
	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMS_MOUSE, sizeof(m), s->t.w[slot].id);
	m.type = type;
	m.x = s->m.pos.x - cl.x;
	m.y = s->m.pos.y - cl.y;
	m.buttons = s->m.buttons;
	m.hit = HT_CLIENT;
	flags = 0;
	if (type == INP_WHEEL)
		m.wheel = (int32_t)extra;
	else
		m.buttons |= (extra & WM_BTN_MASK) << WM_BTN_SHIFT;
	if (type != INP_WHEEL && ((extra & WS_LEAVE)
			|| !rect_contains(cl, s->m.pos)))
		m.hit = HT_NOWHERE;
	if (type == INP_WHEEL || (type == INP_MOUSE_MOVE && !(extra & WS_LEAVE)))
		flags = WSO_DROP;
	ws_post(s, s->t.w[slot].owner, &m, flags);
}

static void	press_visual(t_wsrv *s, int slot, uint32_t ht)
{
	t_wwin		*w;
	uint32_t	shown;

	if (s->m.press_slot < 0)
		return ;
	w = &s->t.w[s->m.press_slot];
	shown = 0;
	if (slot == s->m.press_slot && ht == s->m.press_ht)
		shown = s->m.press_ht;
	if (w->pressed != shown)
		ws_mark_win(s, s->m.press_slot);
	w->pressed = shown;
}

static void	set_hot(t_wsrv *s, int slot, uint32_t ht)
{
	uint32_t	hot;

	hot = 0;
	if (ht == HT_MINBUTTON || ht == HT_MAXBUTTON || ht == HT_CLOSE)
		hot = ht;
	press_visual(s, slot, ht);
	if (s->m.hot_slot >= 0 && (s->m.hot_slot != slot
			|| s->t.w[s->m.hot_slot].hot != hot))
	{
		s->t.w[s->m.hot_slot].hot = 0;
		ws_mark_win(s, s->m.hot_slot);
		s->m.hot_slot = -1;
	}
	if (hot != 0 && slot >= 0 && s->t.w[slot].hot != hot)
	{
		s->t.w[slot].hot = hot;
		s->m.hot_slot = slot;
		ws_mark_win(s, slot);
	}
}

static uint32_t	shape_for(const t_wsrv *s, int slot, uint32_t ht)
{
	const t_wwin	*w;

	if (s->m.capture >= 0)
		return (s->t.w[s->m.capture].cursor);
	if (s->drag.slot >= 0)
		return (s->cur.shape);
	if (slot < 0)
		return (CUR_ARROW);
	w = &s->t.w[slot];
	if (ht == HT_CLIENT)
		return (w->cursor);
	if (wd_is_resize(ht) && (w->style & WS_SIZEBOX)
		&& w->state != WSTATE_MAX)
		return (wd_cursor(ht));
	return (CUR_ARROW);
}

void	ws_hover(t_wsrv *s)
{
	int			slot;
	uint32_t	ht;

	ht = wh_hit(&s->t, s->m.pos, &slot);
	set_hot(s, slot, ht);
	ws_cursor_set(s, s->m.pos, shape_for(s, slot, ht));
	if (s->m.capture >= 0)
	{
		ws_send_mouse(s, s->m.capture, INP_MOUSE_MOVE, 0);
		return ;
	}
	if (s->m.press_slot >= 0 || s->drag.slot >= 0)
		return ;
	if (ht != HT_CLIENT)
		slot = -1;
	if (s->m.hover >= 0 && s->m.hover != slot)
		ws_send_mouse(s, s->m.hover, INP_MOUSE_MOVE, WS_LEAVE);
	s->m.hover = slot;
	if (slot >= 0)
		ws_send_mouse(s, slot, INP_MOUSE_MOVE, 0);
}
