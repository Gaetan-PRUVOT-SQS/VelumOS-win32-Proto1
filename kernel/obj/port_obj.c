#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

static const t_objops	g_listen_ops = {"listener", listener_destroy,
	listener_signaled};

int	listener_new(const char *name, uint32_t len, t_object **out)
{
	t_listener	*l;

	l = kmalloc_tag(sizeof(t_listener), HEAP_IPC);
	if (!l)
		return (E_NOMEM);
	memset(l, 0, sizeof(t_listener));
	memcpy(l->name, name, len);
	l->namelen = len;
	l->obj = obj_create(OBJ_LISTENER, &g_listen_ops, l);
	if (!l->obj)
	{
		kfree(l);
		return (E_NOMEM);
	}
	*out = l->obj;
	return (0);
}

static void	listener_unlink(t_listener *l)
{
	t_listener	**pp;

	if (!l->registered)
		return ;
	pp = &g_ports.head;
	while (*pp && *pp != l)
		pp = &(*pp)->next;
	if (*pp)
		*pp = l->next;
	l->registered = 0;
}

void	listener_destroy(t_object *o)
{
	t_listener	*l;
	t_msgq		local;
	uint64_t	fl;

	l = o->impl;
	fl = spin_lock_irqsave(&g_ports.lock);
	listener_unlink(l);
	local = l->pending;
	memset(&l->pending, 0, sizeof(t_msgq));
	spin_unlock_irqrestore(&g_ports.lock, fl);
	ipc_bury(&local);
	kfree(l);
	ipc_reap();
}

bool	listener_signaled(t_object *o)
{
	t_listener	*l;

	l = o->impl;
	return (__atomic_load_n(&l->pending.count, __ATOMIC_ACQUIRE) > 0);
}
