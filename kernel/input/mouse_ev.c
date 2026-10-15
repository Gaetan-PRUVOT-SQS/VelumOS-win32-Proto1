#include "mouse.h"
#include "velum/libk.h"

static void	event_fill(t_inpevent *ev, uint32_t type, uint32_t code,
		uint32_t mods)
{
	memset(ev, 0, sizeof(*ev));
	ev->type = type;
	ev->code = code;
	ev->mods = mods;
}

static int	button_events(t_mouse *m, uint8_t now, uint32_t mods,
		t_inpevent *out)
{
	uint8_t		changed;
	uint32_t	b;
	int			n;

	changed = (uint8_t)((m->buttons ^ now) & 7);
	n = 0;
	b = 0;
	while (b < 3)
	{
		if (changed & (1u << b))
		{
			if (now & (1u << b))
				event_fill(&out[n], INP_MOUSE_DOWN, b, mods);
			else
				event_fill(&out[n], INP_MOUSE_UP, b, mods);
			n++;
		}
		b++;
	}
	m->buttons = now & 7;
	return (n);
}

int	mouse_events(t_mouse *m, const t_mousepkt *p, uint32_t mods,
		t_inpevent *out)
{
	int32_t	dx;
	int32_t	dy;
	int		n;

	dx = p->dx;
	dy = p->dy;
	accel_apply(&m->accel, &dx, &dy);
	n = 0;
	if (dx != 0 || dy != 0)
	{
		event_fill(&out[n], INP_MOUSE_MOVE, 0, mods);
		out[n].x = dx;
		out[n].y = -dy;
		n++;
	}
	n += button_events(m, p->buttons, mods, &out[n]);
	if (p->dz != 0)
	{
		event_fill(&out[n], INP_WHEEL, 0, mods);
		out[n].y = -p->dz;
		n++;
	}
	return (n);
}
