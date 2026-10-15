#include "velum/libk.h"
#include "ws_srv.h"

static bool	double_click(t_wsrv *s, int slot)
{
	bool	dbl;
	int32_t	dx;
	int32_t	dy;

	dx = s->m.pos.x - s->m.click_pos.x;
	dy = s->m.pos.y - s->m.click_pos.y;
	dbl = (s->m.click_slot == slot && s->now >= s->m.click_ns
			&& s->now - s->m.click_ns <= WS_DBLCLICK_NS
			&& dx >= -WS_DBLCLICK_DIST && dx <= WS_DBLCLICK_DIST
			&& dy >= -WS_DBLCLICK_DIST && dy <= WS_DBLCLICK_DIST);
	s->m.click_slot = slot;
	s->m.click_ns = s->now;
	s->m.click_pos = s->m.pos;
	if (dbl)
		s->m.click_slot = -1;
	return (dbl);
}

static void	toggle_max(t_wsrv *s, int slot)
{
	if (s->t.w[slot].state == WSTATE_MAX)
		ws_set_state(s, slot, WSTATE_NORMAL);
	else
		ws_set_state(s, slot, WSTATE_MAX);
}

void	ws_caption_down(t_wsrv *s, int slot, uint32_t ht)
{
	t_wwin	*w;

	w = &s->t.w[slot];
	if (ht == HT_MINBUTTON || ht == HT_MAXBUTTON || ht == HT_CLOSE)
	{
		s->m.press_slot = slot;
		s->m.press_ht = ht;
		w->pressed = ht;
		ws_mark_win(s, slot);
		return ;
	}
	if (ht == HT_CAPTION && double_click(s, slot))
	{
		if (w->style & WS_MAXBOX)
			toggle_max(s, slot);
		return ;
	}
	if (w->state == WSTATE_MAX)
		return ;
	if (ht == HT_CAPTION
		|| (wd_is_resize(ht) && (w->style & WS_SIZEBOX)))
		ws_drag_begin(s, slot, ht);
}

static void	caption_action(t_wsrv *s, int slot, uint32_t ht)
{
	t_wmhdr	h;
	t_wwin	*w;

	w = &s->t.w[slot];
	if (ht == HT_MINBUTTON && (w->style & WS_MINBOX))
		ws_set_state(s, slot, WSTATE_MIN);
	else if (ht == HT_MAXBUTTON && (w->style & WS_MAXBOX))
		toggle_max(s, slot);
	else if (ht == HT_CLOSE)
	{
		ws_hdr(&h, WMS_CLOSE_REQ, sizeof(h), w->id);
		ws_post(s, w->owner, &h, 0);
	}
}

void	ws_caption_up(t_wsrv *s)
{
	int			slot;
	int			ps;
	uint32_t	pht;
	uint32_t	ht;

	ps = s->m.press_slot;
	pht = s->m.press_ht;
	s->m.press_slot = -1;
	if (ps < 0)
		return ;
	s->t.w[ps].pressed = 0;
	ws_mark_win(s, ps);
	ht = wh_hit(&s->t, s->m.pos, &slot);
	if (slot == ps && ht == pht)
		caption_action(s, ps, pht);
	ws_hover(s);
}
