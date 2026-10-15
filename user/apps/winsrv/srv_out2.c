#include "velum/libk.h"
#include "ws_srv.h"

void	ws_hdr(t_wmhdr *h, uint32_t type, uint32_t size, uint32_t window)
{
	h->magic = WM_MAGIC;
	h->type = type;
	h->size = size;
	h->window = window;
	h->seq = 0;
	h->status = 0;
}

void	ws_flush_all(t_wsrv *s)
{
	int	ci;

	ci = 0;
	while (ci < WS_CLIENT_MAX)
	{
		if (s->c[ci].nout > 0)
			ws_flush(s, ci);
		ci++;
	}
}

bool	ws_pending(const t_wsrv *s)
{
	int	ci;

	ci = 0;
	while (ci < WS_CLIENT_MAX)
	{
		if ((s->c[ci].flags & (WCF_USED | WCF_DEAD)) == WCF_USED
			&& s->c[ci].nout > 0)
			return (true);
		ci++;
	}
	return (false);
}

void	ws_error(t_wsrv *s, int ci, const void *msg, int status)
{
	t_wmhdr	rq;
	t_wmhdr	e;

	memcpy(&rq, msg, sizeof(rq));
	ws_hdr(&e, WMS_ERROR, sizeof(e), rq.window);
	e.seq = rq.seq;
	e.status = status;
	ws_post(s, ci, &e, 0);
}
