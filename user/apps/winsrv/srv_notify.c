#include "velum/libk.h"
#include "ws_srv.h"

bool	ws_listed(const t_wwin *w)
{
	return (w->id != 0 && !(w->style
			& (WS_TOOLWINDOW | WS_DESKTOP | WS_APPBAR | WS_POPUP)));
}

void	ws_notify(t_wsrv *s, int slot, uint32_t type)
{
	t_wminfo	m;
	t_wwin		*w;

	w = &s->t.w[slot];
	if (s->shell < 0 || !(s->c[s->shell].flags & WCF_SUB) || !ws_listed(w))
		return ;
	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, type, sizeof(m), w->id);
	m.pid = s->c[w->owner].serial;
	m.style = w->style;
	m.state = w->state;
	m.icon = w->icon;
	m.active = (s->t.active == slot);
	memcpy(m.title, w->title, sizeof(m.title));
	ws_post(s, s->shell, &m, 0);
}

void	ws_notify_all(t_wsrv *s)
{
	uint32_t	i;

	i = 0;
	while (i < s->t.nz)
	{
		ws_notify(s, s->t.z[i], WMS_WIN_ADD);
		i++;
	}
}
