#include <string.h>
#include "velum/err.h"
#include "fake_kern.h"
#include "ws_sys.h"

int64_t	ws_sys_listen(const char *name)
{
	int		obj;
	int64_t	h;

	if (name == NULL || g_fk.listener >= 0)
		return (E_EXIST);
	obj = fk_obj_new(FK_LISTEN);
	if (obj < 0)
		return (obj);
	h = fk_handle_new(obj);
	if (h < 0)
	{
		fk_unref(obj);
		return (h);
	}
	g_fk.listener = obj;
	return (h);
}

int64_t	ws_sys_accept(t_handle listener)
{
	int		obj;
	t_fkobj	*o;
	int64_t	h;

	obj = fk_obj_of(listener, FK_LISTEN);
	if (obj < 0)
		return (obj);
	o = &g_fk.o[obj];
	if (o->npend == 0)
		return (E_TIMEOUT);
	obj = o->pend[0];
	o->npend--;
	memmove(&o->pend[0], &o->pend[1], o->npend * sizeof(o->pend[0]));
	h = fk_handle_new(obj);
	if (h < 0)
		fk_unref(obj);
	return (h);
}

int	ws_sys_send(t_handle ch, const void *msg, uint32_t len, t_handle h)
{
	return (fk_send(ch, msg, len, h));
}

int	ws_sys_recv(t_handle ch, t_wsrx *rx)
{
	t_handle	hx;
	int			r;

	r = fk_recv(ch, rx->buf, sizeof(rx->buf), &hx);
	rx->nhandles = (hx != 0);
	rx->handles[0] = hx;
	if (r >= 0)
		rx->len = (uint32_t)r;
	return (r);
}

int	ws_sys_close(t_handle h)
{
	if (h == 0)
		return (0);
	return (fk_close(h));
}
