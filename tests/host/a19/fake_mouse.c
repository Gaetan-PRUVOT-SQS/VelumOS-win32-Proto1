#include <string.h>
#include "fake.h"

uint32_t	g_fake_buttons;
int32_t		g_fake_wheel;

bool	fake_mouse(t_ctlroot *r, uint32_t type, int x, int y)
{
	t_wmmouse	m;

	memset(&m, 0, sizeof(m));
	m.type = type;
	m.x = x;
	m.y = y;
	m.buttons = g_fake_buttons;
	m.wheel = g_fake_wheel;
	g_fake_wheel = 0;
	return (ctl_mouse(r, &m));
}

bool	fake_down(t_ctlroot *r, int x, int y)
{
	g_fake_buttons |= 1u << BTN_LEFT;
	return (fake_mouse(r, INP_MOUSE_DOWN, x, y));
}

bool	fake_up(t_ctlroot *r, int x, int y)
{
	g_fake_buttons &= ~(1u << BTN_LEFT);
	return (fake_mouse(r, INP_MOUSE_UP, x, y));
}

bool	fake_move(t_ctlroot *r, int x, int y)
{
	return (fake_mouse(r, INP_MOUSE_MOVE, x, y));
}

bool	fake_wheel(t_ctlroot *r, int x, int y, int delta)
{
	g_fake_wheel = delta;
	return (fake_mouse(r, INP_WHEEL, x, y));
}
