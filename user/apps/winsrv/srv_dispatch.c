#include "velum/err.h"
#include "velum/libk.h"
#include "ws_srv.h"

static bool	admissible(t_wsrv *s, int ci, t_wsrx *rx)
{
	t_wmhdr		h;
	uint32_t	i;

	i = 0;
	while (i < rx->nhandles && i < IPC_HANDLES_MAX)
		ws_sys_close(rx->handles[i++]);
	if (wsp_validate(rx->buf, rx->len, rx->nhandles) == 0)
	{
		memcpy(&h, rx->buf, sizeof(h));
		if ((h.type == WMC_HELLO) != ((s->c[ci].flags & WCF_HELLO) != 0))
			return (true);
	}
	s->c[ci].flags |= WCF_DEAD;
	s->violations++;
	ws_sys_log("winsrv: client deconnecte (protocole)");
	return (false);
}

void	ws_dispatch(t_wsrv *s, int ci, t_wsrx *rx)
{
	static const t_wsreq	req[] = {NULL, ws_req_hello, ws_req_create,
		ws_req_destroy, ws_req_title, ws_req_rect, ws_req_state,
		ws_req_present, ws_req_cursor, ws_req_activate, ws_req_workarea,
		ws_req_subscribe, ws_req_icon, ws_req_capture};
	t_wmhdr					h;
	int						r;

	if (!admissible(s, ci, rx))
		return ;
	memcpy(&h, rx->buf, sizeof(h));
	r = req[h.type](s, ci, rx->buf);
	if (r == E_PROTO)
	{
		s->c[ci].flags |= WCF_DEAD;
		s->violations++;
		ws_sys_log("winsrv: client deconnecte (fenetre etrangere)");
	}
	else if (r < 0)
		ws_error(s, ci, rx->buf, r);
}

int	ws_own(const t_wsrv *s, int ci, uint32_t id)
{
	int	slot;

	slot = wt_find(&s->t, id);
	if (slot < 0 || s->t.w[slot].owner != ci)
		return (E_PROTO);
	return (slot);
}

int	ws_target(const t_wsrv *s, int ci, uint32_t id)
{
	int	slot;

	slot = wt_find(&s->t, id);
	if (slot >= 0 && s->t.w[slot].owner == ci)
		return (slot);
	if (ci != s->shell)
		return (E_PROTO);
	if (slot < 0)
		return (E_NOENT);
	return (slot);
}

int	ws_claim_shell(t_wsrv *s, int ci)
{
	if (s->shell < 0)
		s->shell = ci;
	if (s->shell != ci)
		return (E_PERM);
	return (0);
}
