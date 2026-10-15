#include "help.h"

void	hs_click(t_wsrv *s, int32_t x, int32_t y)
{
	hs_point(s, x, y);
	fk_push_input(INP_MOUSE_DOWN, BTN_LEFT, 0, 0);
	fk_push_input(INP_MOUSE_UP, BTN_LEFT, 0, 0);
	hs_input(s);
}

void	hs_drag(t_wsrv *s, t_point from, t_point to)
{
	hs_point(s, from.x, from.y);
	fk_push_input(INP_MOUSE_DOWN, BTN_LEFT, 0, 0);
	hs_input(s);
	hs_point(s, (from.x + to.x) / 2, (from.y + to.y) / 2);
	hs_point(s, to.x, to.y);
	fk_push_input(INP_MOUSE_UP, BTN_LEFT, 0, 0);
	hs_input(s);
}

void	hs_key(t_wsrv *s, uint32_t vk, uint32_t mods)
{
	fk_push_key(INP_KEY_DOWN, vk, mods);
	fk_push_key(INP_KEY_UP, vk, mods);
	hs_input(s);
}

int	hs_slot(const t_wsrv *s, uint32_t id)
{
	return (wt_find(&s->t, id));
}

uint32_t	hs_create(t_wsrv *s, t_handle c, t_rect r, uint32_t style)
{
	uint32_t	id;

	id = hr_create(s, c, r, style);
	hs_settle(s, c);
	return (id);
}
