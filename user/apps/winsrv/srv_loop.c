#include "velum/abi/abi_syscall.h"
#include "velum/err.h"
#include "ws_srv.h"

static uint32_t	fixed_sources(t_wsrv *s, t_handle *hs, int32_t *src)
{
	uint32_t	n;

	n = 0;
	if (s->input != 0)
	{
		hs[n] = s->input;
		src[n] = WS_SRC_INPUT;
		n++;
	}
	if (s->listener != 0)
	{
		hs[n] = s->listener;
		src[n] = WS_SRC_LISTEN;
		n++;
	}
	return (n);
}

static uint32_t	build(t_wsrv *s, t_handle *hs, int32_t *src)
{
	uint32_t	n;
	uint32_t	k;
	int			ci;

	n = fixed_sources(s, hs, src);
	k = 0;
	while (k < WS_CLIENT_MAX)
	{
		ci = (int)((k + s->rot) % WS_CLIENT_MAX);
		if ((s->c[ci].flags & (WCF_USED | WCF_DEAD)) == WCF_USED)
		{
			hs[n] = s->c[ci].chan;
			src[n] = ci;
			n++;
		}
		k++;
	}
	s->rot++;
	return (n);
}

uint64_t	ws_timeout(const t_wsrv *s)
{
	if (s->dirty.n > 0)
	{
		if (s->now >= s->next_frame)
			return (0);
		return (s->next_frame - s->now);
	}
	if (ws_pending(s))
		return (WS_FRAME_NS);
	return (TIMEOUT_INF);
}

static void	serve(t_wsrv *s, int32_t src)
{
	if (src == WS_SRC_INPUT)
		ws_input_read(s);
	else if (src == WS_SRC_LISTEN)
		ws_accept(s);
	else if (src >= 0 && src < WS_CLIENT_MAX)
		ws_client_read(s, src);
}

int	ws_step(t_wsrv *s)
{
	t_handle	hs[2 + WS_CLIENT_MAX];
	int32_t		src[2 + WS_CLIENT_MAX];
	uint32_t	n;
	int			r;

	n = build(s, hs, src);
	s->now = ws_sys_now();
	r = ws_sys_wait(hs, n, ws_timeout(s));
	s->now = ws_sys_now();
	if (r >= 0 && (uint32_t)r < n)
		serve(s, src[r]);
	ws_flush_all(s);
	ws_reap(s);
	if (s->dirty.n > 0 && s->now >= s->next_frame)
		ws_frame(s);
	if (r < 0 && r != E_TIMEOUT)
		return (r);
	return (0);
}
