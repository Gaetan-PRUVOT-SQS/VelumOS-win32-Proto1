#include "velum/libk.h"
#include "velum/luna.h"
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

static uint32_t	g_theme_dark;
static uint32_t	g_theme_reduced;

static void	theme_fill(t_wmtheme *m)
{
	memset(m, 0, sizeof(*m));
	ws_hdr(&m->h, WMS_THEME, sizeof(*m), 0);
	m->dark = g_theme_dark;
	m->reduced = g_theme_reduced;
}

void	ws_theme_send(t_wsrv *s, int ci)
{
	t_wmtheme	m;

	if (g_theme_dark == 0 && g_theme_reduced == 0)
		return ;
	theme_fill(&m);
	ws_post(s, ci, &m, 0);
}

void	ws_theme_set(t_wsrv *s, uint32_t dark, uint32_t reduced)
{
	t_wmtheme	m;
	int			ci;

	if (dark == g_theme_dark && reduced == g_theme_reduced)
		return ;
	g_theme_dark = dark;
	g_theme_reduced = reduced;
	luna_theme_set(dark ? LUNA_THEME_DARK : LUNA_THEME_LIGHT);
	luna_transparency_set(reduced != 0);
	theme_fill(&m);
	ci = 0;
	while (ci < WS_CLIENT_MAX)
	{
		if ((s->c[ci].flags & (WCF_USED | WCF_HELLO | WCF_DEAD))
			== (WCF_USED | WCF_HELLO))
			ws_post(s, ci, &m, 0);
		ci++;
	}
	ws_mark(s, s->t.screen);
}

void	ws_theme_reset(void)
{
	g_theme_dark = 0;
	g_theme_reduced = 0;
}
