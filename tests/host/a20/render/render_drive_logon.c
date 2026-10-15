#include "velum/abi/abi_input.h"
#include "render.h"

void	lgd_click(t_logon *lg, t_point p)
{
	t_uimsg	m;

	msg_button(&m, lg->ui.win.id, INP_MOUSE_DOWN, p);
	logon_event(lg, &m);
	msg_button(&m, lg->ui.win.id, INP_MOUSE_UP, p);
	logon_event(lg, &m);
}

void	lgd_key(t_logon *lg, uint32_t code)
{
	t_uimsg	m;

	msg_key(&m, lg->ui.win.id, INP_KEY_DOWN, code);
	logon_event(lg, &m);
	msg_key(&m, lg->ui.win.id, INP_KEY_UP, code);
	logon_event(lg, &m);
}

void	lgd_type(t_logon *lg, const char *text)
{
	t_uimsg	m;

	while (*text)
	{
		msg_key(&m, lg->ui.win.id, INP_CHAR, (uint8_t)(*text));
		logon_event(lg, &m);
		text++;
	}
}

t_point	lgd_tile(const t_logon *lg, uint32_t i)
{
	t_rect	r;

	r = lg->lay.tile[i];
	return ((t_point){lg->lay.card.x + r.x + r.w / 2,
		lg->lay.card.y + r.y + r.h / 2});
}

void	lgd_press_shutdown(t_logon *lg)
{
	t_rect	shut;

	shut = lg->lay.shutdown;
	lgd_click(lg, (t_point){lg->lay.card.x + shut.x + 5, lg->lay.card.y + shut.y
		+ 5});
}
