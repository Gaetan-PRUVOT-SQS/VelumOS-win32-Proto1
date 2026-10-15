#include "ws_srv.h"

static void	click_activate(t_wsrv *s, int slot)
{
	t_wwin	*w;

	w = &s->t.w[slot];
	if (wf_activatable(w))
		ws_activate(s, slot);
	else if (!(w->style & (WS_NOACTIVATE | WS_DESKTOP))
		&& wz_raise(&s->t, slot))
		ws_mark_win(s, slot);
}

static void	button_up(t_wsrv *s, uint32_t button)
{
	if (button == BTN_LEFT && s->drag.slot >= 0)
	{
		ws_drag_end(s);
		return ;
	}
	if (button == BTN_LEFT && s->m.press_slot >= 0)
	{
		ws_caption_up(s);
		return ;
	}
	if (s->m.capture < 0)
		return ;
	ws_send_mouse(s, s->m.capture, INP_MOUSE_UP, button);
	if (!s->m.explicit_cap && s->m.buttons == 0)
	{
		s->m.capture = -1;
		ws_hover(s);
	}
}

static void	pointer_down(t_wsrv *s, int slot, uint32_t ht, uint32_t button)
{
	if (ht == HT_CLIENT)
	{
		s->m.capture = slot;
		s->m.explicit_cap = false;
		if (s->m.hover >= 0 && s->m.hover != slot)
			ws_send_mouse(s, s->m.hover, INP_MOUSE_MOVE, WS_LEAVE);
		s->m.hover = slot;
		ws_send_mouse(s, slot, INP_MOUSE_DOWN, button);
		return ;
	}
	if (button == BTN_LEFT)
		ws_caption_down(s, slot, ht);
}

void	ws_mouse_button(t_wsrv *s, uint32_t button, bool down)
{
	int			slot;
	uint32_t	ht;

	if (button > BTN_MIDDLE)
		return ;
	if (!down)
	{
		s->m.buttons &= ~(1u << button);
		button_up(s, button);
		return ;
	}
	s->m.buttons |= 1u << button;
	if (s->drag.slot >= 0 || s->m.press_slot >= 0)
		return ;
	if (s->m.capture >= 0)
	{
		ws_send_mouse(s, s->m.capture, INP_MOUSE_DOWN, button);
		return ;
	}
	ht = wh_hit(&s->t, s->m.pos, &slot);
	if (slot < 0)
		return ;
	click_activate(s, slot);
	pointer_down(s, slot, ht, button);
}

void	ws_mouse_wheel(t_wsrv *s, int32_t delta)
{
	int			slot;
	uint32_t	ht;

	if (delta == 0)
		return ;
	slot = s->m.capture;
	if (slot < 0)
	{
		ht = wh_hit(&s->t, s->m.pos, &slot);
		if (ht != HT_CLIENT)
			return ;
	}
	ws_send_mouse(s, slot, INP_WHEEL, (uint32_t)delta);
}
