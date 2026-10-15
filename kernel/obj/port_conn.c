#include "obj_int.h"
#include "velum/heap.h"

static int	conn_enqueue(const char *name, uint32_t len, t_ipcmsg *m,
		t_object **lobj)
{
	t_listener	*l;
	uint64_t	fl;
	int			rc;

	*lobj = NULL;
	fl = spin_lock_irqsave(&g_ports.lock);
	l = port_find(name, len);
	rc = E_NOENT;
	if (l && obj_tryref(l->obj))
	{
		*lobj = l->obj;
		rc = msgq_push(&l->pending, m, IPC_QUEUE_MAX);
	}
	spin_unlock_irqrestore(&g_ports.lock, fl);
	return (rc);
}

static void	conn_undo(t_ipcmsg *m, t_object **client)
{
	msg_free(m);
	obj_unref(*client);
	*client = NULL;
}

int	port_connect(const char *name, uint32_t len, t_object **out)
{
	t_object	*srv;
	t_object	*lobj;
	t_ipcmsg	*m;
	int			rc;

	if (!port_name_ok(name, len) || !out)
		return (E_INVAL);
	m = msg_alloc(0);
	if (!m)
		return (E_NOMEM);
	rc = chan_create(out, &srv);
	if (rc < 0)
	{
		kfree(m);
		return (rc);
	}
	m->objs[0] = srv;
	m->nh = 1;
	rc = conn_enqueue(name, len, m, &lobj);
	if (rc == 0)
		obj_signal(lobj);
	obj_unref(lobj);
	if (rc < 0)
		conn_undo(m, out);
	return (rc);
}

static t_ipcmsg	*accept_pop(t_listener *l)
{
	uint64_t	fl;
	t_ipcmsg	*m;

	fl = spin_lock_irqsave(&g_ports.lock);
	m = msgq_pop(&l->pending);
	spin_unlock_irqrestore(&g_ports.lock, fl);
	return (m);
}

int	port_accept(t_object *lobj, uint64_t deadline, t_object **out)
{
	t_ipcmsg	*m;
	int			wrc;

	if (!lobj || lobj->type != OBJ_LISTENER || !out)
		return (E_INVAL);
	m = accept_pop(lobj->impl);
	wrc = 0;
	while (!m && wrc == 0)
	{
		wrc = wait_for(lobj, deadline);
		m = accept_pop(lobj->impl);
	}
	if (!m)
		return (wrc);
	*out = m->objs[0];
	m->nh = 0;
	msg_free(m);
	return (0);
}
