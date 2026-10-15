#include <stdlib.h>
#include <string.h>
#include "velum/err.h"
#include "fake_kern.h"

int	fk_close(t_handle h)
{
	int	obj;

	obj = fk_obj_of(h, FK_FREE);
	if (obj < 0)
		return (obj);
	g_fk.h[h - 1] = 0;
	fk_unref(obj);
	return (0);
}

static int	push(t_fkobj *to, const void *msg, uint32_t len, int32_t xobj)
{
	t_fkmsg	*m;

	m = &to->q[(to->head + to->n) % IPC_QUEUE_MAX];
	m->data = malloc(len + 1);
	if (m->data == NULL)
		return (E_NOMEM);
	memcpy(m->data, msg, len);
	m->len = len;
	m->obj = xobj;
	if (xobj >= 0)
		g_fk.o[xobj].refs++;
	to->n++;
	return (0);
}

int	fk_send(t_handle ch, const void *msg, uint32_t len, t_handle hx)
{
	int		obj;
	int		xobj;
	t_fkobj	*to;

	obj = fk_obj_of(ch, FK_CHAN);
	if (obj < 0)
		return (obj);
	if (len > IPC_MSG_MAX || msg == NULL)
		return (E_INVAL);
	xobj = -1;
	if (hx != 0)
		xobj = fk_obj_of(hx, FK_FREE);
	if (hx != 0 && xobj < 0)
		return (xobj);
	if (g_fk.o[obj].peer < 0)
		return (E_PIPE);
	to = &g_fk.o[g_fk.o[obj].peer];
	if (to->n >= IPC_QUEUE_MAX)
		return (E_AGAIN);
	return (push(to, msg, len, xobj));
}

static int	deliver(t_fkmsg *m, void *buf, uint32_t max, t_handle *hx)
{
	int64_t	h;

	if (m->obj >= 0)
	{
		h = fk_handle_new(m->obj);
		if (h > 0)
			*hx = (t_handle)h;
		else
			fk_unref(m->obj);
	}
	if (m->len > max)
	{
		free(m->data);
		return (E_OVERFLOW);
	}
	memcpy(buf, m->data, m->len);
	free(m->data);
	return ((int)m->len);
}

int	fk_recv(t_handle ch, void *buf, uint32_t max, t_handle *hx)
{
	int		obj;
	t_fkobj	*o;
	t_fkmsg	m;

	obj = fk_obj_of(ch, FK_CHAN);
	if (obj < 0)
		return (obj);
	o = &g_fk.o[obj];
	*hx = 0;
	if (o->n == 0 && o->peer < 0)
		return (E_PIPE);
	if (o->n == 0)
		return (E_TIMEOUT);
	m = o->q[o->head];
	o->head = (o->head + 1) % IPC_QUEUE_MAX;
	o->n--;
	return (deliver(&m, buf, max, hx));
}
