#include <string.h>
#include "velum/err.h"
#include "help.h"

t_hrlast	g_hr;

t_handle	hr_connect(t_wsrv *s)
{
	int64_t	h;

	h = fk_connect();
	if (h <= 0)
		return (0);
	ws_step(s);
	return ((t_handle)h);
}

int	hr_send(t_wsrv *s, t_handle c, void *msg)
{
	t_wmhdr	h;
	int		r;

	memcpy(&h, msg, sizeof(h));
	r = fk_send(c, msg, h.size, 0);
	hs_settle(s, c);
	return (r);
}

int	hr_recv(t_handle c, void *buf, t_handle *hx)
{
	t_handle	dummy;

	if (hx == NULL)
		hx = &dummy;
	return (fk_recv(c, buf, WM_MSG_MAX, hx));
}

int	hr_hello(t_wsrv *s, t_handle c)
{
	t_wmhello	m;
	t_handle	hx;

	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMC_HELLO, sizeof(m), 0);
	m.version = WM_VERSION;
	m.h.seq = 1;
	hr_send(s, c, &m);
	if (hr_recv(c, &m, &hx) < 0 || m.h.type != WMS_HELLO_OK)
		return (E_PROTO);
	return (0);
}

uint32_t	hr_create(t_wsrv *s, t_handle c, t_rect r, uint32_t style)
{
	t_wmcreate	m;
	uint64_t	buf[WM_MSG_MAX / 8];
	t_wmhdr		h;
	t_handle	hx;

	memset(&m, 0, sizeof(m));
	ws_hdr(&m.h, WMC_CREATE, sizeof(m), 0);
	m.h.seq = 7;
	m.rect = r;
	m.style = style;
	memcpy(m.title, "Essai", 6);
	hr_send(s, c, &m);
	while (hr_recv(c, buf, &hx) >= 0)
	{
		memcpy(&h, buf, sizeof(h));
		if (hx != 0)
			fk_close(hx);
		if (h.type == WMS_CREATED)
			return (h.window);
		g_hr.status = h.status;
		g_hr.seq = h.seq;
		if (h.type == WMS_ERROR)
			return (0);
	}
	return (0);
}
