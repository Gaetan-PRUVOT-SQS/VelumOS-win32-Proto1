#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

static int	free_client(const t_wsrv *s)
{
	int	ci;

	ci = 0;
	while (ci < WS_CLIENT_MAX && (s->c[ci].flags & WCF_USED))
		ci++;
	if (ci == WS_CLIENT_MAX)
		return (-1);
	return (ci);
}

void	ws_accept(t_wsrv *s)
{
	int64_t	h;
	int		ci;
	int		n;

	n = 0;
	while (n < 8)
	{
		h = ws_sys_accept(s->listener);
		if (h <= 0)
			return ;
		ci = free_client(s);
		if (ci < 0)
		{
			ws_sys_close((t_handle)h);
			ws_sys_log("winsrv: trop de clients, connexion refusee");
			return ;
		}
		memset(&s->c[ci], 0, sizeof(s->c[ci]));
		s->c[ci].chan = (t_handle)h;
		s->c[ci].flags = WCF_USED;
		s->serial++;
		s->c[ci].serial = s->serial;
		n++;
	}
}

void	ws_client_read(t_wsrv *s, int ci)
{
	int	i;
	int	r;

	i = 0;
	while (i < WS_RECV_BATCH
		&& (s->c[ci].flags & (WCF_USED | WCF_DEAD)) == WCF_USED)
	{
		r = ws_sys_recv(s->c[ci].chan, &s->rx);
		if (r == E_TIMEOUT || r == E_AGAIN)
			return ;
		if (r < 0)
		{
			s->c[ci].flags |= WCF_DEAD;
			return ;
		}
		ws_dispatch(s, ci, &s->rx);
		i++;
	}
}

void	ws_client_drop(t_wsrv *s, int ci)
{
	int	slot;

	if (!(s->c[ci].flags & WCF_USED))
		return ;
	slot = 0;
	while (slot < WS_WIN_MAX)
	{
		if (s->t.w[slot].id != 0 && s->t.w[slot].owner == ci)
			ws_win_destroy(s, slot);
		slot++;
	}
	ws_sys_close(s->c[ci].chan);
	memset(&s->c[ci], 0, sizeof(s->c[ci]));
	if (s->shell == ci)
	{
		s->shell = -1;
		s->work_set = rect_make(0, 0, 0, 0);
		ws_workarea(s);
	}
}

void	ws_reap(t_wsrv *s)
{
	int	ci;

	ci = 0;
	while (ci < WS_CLIENT_MAX)
	{
		if ((s->c[ci].flags & (WCF_USED | WCF_DEAD)) == (WCF_USED | WCF_DEAD))
			ws_client_drop(s, ci);
		ci++;
	}
}
