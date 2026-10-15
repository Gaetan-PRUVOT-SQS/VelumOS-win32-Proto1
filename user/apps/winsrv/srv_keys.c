#include "velum/libk.h"
#include "ws_srv.h"

static int	shell_window(const t_wsrv *s)
{
	int	i;
	int	any;

	any = -1;
	i = 0;
	while (s->shell >= 0 && i < WS_WIN_MAX)
	{
		if (s->t.w[i].id != 0 && s->t.w[i].owner == s->shell)
		{
			if (s->t.w[i].style & WS_APPBAR)
				return (i);
			if (any < 0 || (s->t.w[i].style & WS_DESKTOP))
				any = i;
		}
		i++;
	}
	return (any);
}

static void	send_key(t_wsrv *s, int slot, const t_inpevent *ev)
{
	t_wmkey	k;

	if (slot < 0)
		return ;
	memset(&k, 0, sizeof(k));
	ws_hdr(&k.h, WMS_KEY, sizeof(k), s->t.w[slot].id);
	k.ev = *ev;
	ws_post(s, s->t.w[slot].owner, &k, 0);
}

static bool	server_key(t_wsrv *s, const t_inpevent *ev)
{
	t_wmhdr	h;
	int		a;

	if (ev->type == INP_CHAR || !(ev->mods & INPM_ALT))
		return (false);
	if (ev->code == VK_TAB)
	{
		if (ev->type == INP_KEY_DOWN)
			ws_alttab(s, (ev->mods & INPM_SHIFT) != 0);
		return (true);
	}
	a = s->t.active;
	if (ev->code != VK_F1 + 3 || (ev->mods & INPM_CTRL) || a < 0)
		return (false);
	if (ev->type == INP_KEY_DOWN)
	{
		ws_hdr(&h, WMS_CLOSE_REQ, sizeof(h), s->t.w[a].id);
		ws_post(s, s->t.w[a].owner, &h, 0);
	}
	return (true);
}

void	ws_key(t_wsrv *s, const t_inpevent *ev)
{
	if (s->k.n > 0 && (!(ev->mods & INPM_ALT) || (ev->type == INP_KEY_UP
				&& (ev->code == VK_MENU || ev->code == VK_LMENU
					|| ev->code == VK_RMENU))))
		s->k.n = 0;
	if (server_key(s, ev))
		return ;
	if (ev->type != INP_CHAR && (ev->code == VK_LWIN || ev->code == VK_RWIN))
	{
		send_key(s, shell_window(s), ev);
		return ;
	}
	if (s->m.capture >= 0 && s->m.explicit_cap)
		send_key(s, s->m.capture, ev);
	else
		send_key(s, s->t.active, ev);
}

void	ws_alttab(t_wsrv *s, bool back)
{
	uint32_t	i;
	int			slot;

	if (s->k.n == 0)
	{
		i = s->t.nz;
		while (i-- > 0)
		{
			slot = s->t.z[i];
			if (ws_listed(&s->t.w[slot]) && (wf_activatable(&s->t.w[slot])
					|| s->t.w[slot].state == WSTATE_MIN))
				s->k.ids[s->k.n++] = s->t.w[slot].id;
		}
		s->k.pos = 0;
	}
	if (s->k.n < 2)
		return ;
	if (back)
		s->k.pos = (s->k.pos + s->k.n - 1) % s->k.n;
	else
		s->k.pos = (s->k.pos + 1) % s->k.n;
	slot = wt_find(&s->t, s->k.ids[s->k.pos]);
	if (slot >= 0)
		ws_activate(s, slot);
}
