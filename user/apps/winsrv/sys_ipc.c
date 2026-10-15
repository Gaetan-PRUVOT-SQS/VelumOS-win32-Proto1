#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vobj.h"
#include "ws_sys.h"

int64_t	ws_sys_listen(const char *name)
{
	return (v_port_listen(name, 0));
}

int64_t	ws_sys_accept(t_handle listener)
{
	return (v_port_accept(listener, 0));
}

int	ws_sys_send(t_handle ch, const void *msg, uint32_t len, t_handle h)
{
	t_chansend	s;
	t_handle	hs[1];

	hs[0] = h;
	s.msg = (uint64_t)(uintptr_t)msg;
	s.len = len;
	s.handles = (uint64_t)(uintptr_t)hs;
	s.nhandles = (h != 0);
	s.reserved = 0;
	return (v_chan_send(ch, &s));
}

int	ws_sys_recv(t_handle ch, t_wsrx *rx)
{
	t_chanrecv	r;
	int			ret;

	memset(&r, 0, sizeof(r));
	r.buf = (uint64_t)(uintptr_t)rx->buf;
	r.buf_len = sizeof(rx->buf);
	r.handles = (uint64_t)(uintptr_t)rx->handles;
	r.handles_max = IPC_HANDLES_MAX;
	r.timeout_ns = 0;
	rx->len = 0;
	rx->nhandles = 0;
	ret = v_chan_recv(ch, &r);
	if (ret < 0)
		return (ret);
	rx->nhandles = r.out_nhandles;
	if (rx->nhandles > IPC_HANDLES_MAX)
		rx->nhandles = IPC_HANDLES_MAX;
	rx->len = (uint32_t)r.out_len;
	if (r.out_len > sizeof(rx->buf))
		rx->len = 0;
	return ((int)rx->len);
}

int	ws_sys_close(t_handle h)
{
	if (h == 0)
		return (0);
	return (v_close(h));
}
