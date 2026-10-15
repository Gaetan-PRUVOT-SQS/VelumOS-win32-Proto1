#include "velum/err.h"
#include "velum/libk.h"
#include "velum/vobj.h"
#include "velum/vtime.h"
#include "wmc_int.h"

int64_t	wmsys_connect(const char *name)
{
	return (v_port_connect(name, 0));
}

int	wmsys_send(t_handle ch, const void *msg, uint32_t len)
{
	t_chansend	s;

	s.msg = (uint64_t)(uintptr_t)msg;
	s.len = len;
	s.handles = 0;
	s.nhandles = 0;
	s.reserved = 0;
	return (v_chan_send(ch, &s));
}

static int	finish(t_wmcmsg *m, const t_chanrecv *r, const t_handle *hs)
{
	uint32_t	i;
	uint32_t	n;

	n = r->out_nhandles;
	if (n > IPC_HANDLES_MAX)
		n = IPC_HANDLES_MAX;
	i = 1;
	while (i < n)
		v_close(hs[i++]);
	m->handle = 0;
	if (n > 0)
		m->handle = hs[0];
	if (r->out_len > sizeof(m->buf))
	{
		if (m->handle != 0)
			v_close(m->handle);
		m->handle = 0;
		return (E_PROTO);
	}
	m->len = (uint32_t)r->out_len;
	return ((int)m->len);
}

int	wmsys_recv(t_handle ch, t_wmcmsg *m, uint64_t timeout_ns)
{
	t_chanrecv	r;
	t_handle	hs[IPC_HANDLES_MAX];
	int			ret;

	memset(&r, 0, sizeof(r));
	r.buf = (uint64_t)(uintptr_t)m->buf;
	r.buf_len = sizeof(m->buf);
	r.handles = (uint64_t)(uintptr_t)hs;
	r.handles_max = IPC_HANDLES_MAX;
	r.timeout_ns = timeout_ns;
	m->handle = 0;
	m->len = 0;
	ret = v_chan_recv(ch, &r);
	if (ret < 0)
		return (ret);
	return (finish(m, &r, hs));
}

uint64_t	wmsys_now(void)
{
	int64_t	t;

	t = v_time_mono();
	if (t < 0)
		return (0);
	return ((uint64_t)t);
}
