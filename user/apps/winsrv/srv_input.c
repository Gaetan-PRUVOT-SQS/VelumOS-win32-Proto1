#include "ws_srv.h"

void	ws_input(t_wsrv *s, const t_inpevent *ev)
{
	if (ev->type == INP_MOUSE_MOVE)
		ws_mouse_move(s, ev->x, ev->y);
	else if (ev->type == INP_MOUSE_DOWN || ev->type == INP_MOUSE_UP)
		ws_mouse_button(s, ev->code, ev->type == INP_MOUSE_DOWN);
	else if (ev->type == INP_WHEEL)
		ws_mouse_wheel(s, ev->y);
	else if (ev->type == INP_KEY_DOWN || ev->type == INP_KEY_UP
		|| ev->type == INP_CHAR)
		ws_key(s, ev);
}

void	ws_input_read(t_wsrv *s)
{
	t_inpevent	ev[WS_INPUT_BATCH];
	int			n;
	int			i;
	int			rounds;

	rounds = 0;
	while (s->input != 0 && rounds < 4)
	{
		n = ws_sys_input_read(s->input, ev, WS_INPUT_BATCH);
		if (n <= 0)
			return ;
		i = 0;
		while (i < n && i < WS_INPUT_BATCH)
		{
			ws_input(s, &ev[i]);
			i++;
		}
		if (n < WS_INPUT_BATCH)
			return ;
		rounds++;
	}
}

void	ws_drag_begin(t_wsrv *s, int slot, uint32_t ht)
{
	wd_begin(&s->drag, slot, ht, s->m.pos);
	s->drag.orig = s->t.w[slot].rect;
}

void	ws_drag_end(t_wsrv *s)
{
	int	slot;

	slot = s->drag.slot;
	if (slot < 0)
		return ;
	s->drag.slot = -1;
	ws_apply_rect(s, slot, s->t.w[slot].rect, true);
	ws_hover(s);
}

void	ws_mouse_move(t_wsrv *s, int32_t dx, int32_t dy)
{
	t_point	p;
	t_rect	sc;

	sc = s->t.screen;
	p.x = wg_clamp(s->m.pos.x + wg_clamp(dx, -WS_COORD_MAX, WS_COORD_MAX),
			sc.x, sc.x + sc.w - 1);
	p.y = wg_clamp(s->m.pos.y + wg_clamp(dy, -WS_COORD_MAX, WS_COORD_MAX),
			sc.y, sc.y + sc.h - 1);
	if (p.x == s->m.pos.x && p.y == s->m.pos.y)
		return ;
	s->m.pos = p;
	if (s->drag.slot >= 0)
	{
		ws_apply_rect(s, s->drag.slot, wd_target(&s->drag, &s->t, p), false);
		ws_cursor_set(s, p, s->cur.shape);
		return ;
	}
	ws_hover(s);
}
