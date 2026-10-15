#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

static const t_objops	g_chan_ops = {"channel", chan_destroy, chan_signaled};

static int	chan_create_fail(t_chanpair *p)
{
	kfree(p->ends[0]);
	kfree(p->ends[1]);
	kfree(p);
	return (E_NOMEM);
}

int	chan_create(t_object **a, t_object **b)
{
	t_chanpair	*p;

	p = kmalloc_tag(sizeof(t_chanpair), HEAP_IPC);
	if (!p)
		return (E_NOMEM);
	memset(p, 0, sizeof(t_chanpair));
	spin_init(&p->lock, "chan");
	p->impl[0].pair = p;
	p->impl[1].pair = p;
	p->impl[1].side = 1;
	p->ends[0] = obj_create(OBJ_CHANNEL, &g_chan_ops, &p->impl[0]);
	p->ends[1] = obj_create(OBJ_CHANNEL, &g_chan_ops, &p->impl[1]);
	if (!p->ends[0] || !p->ends[1])
		return (chan_create_fail(p));
	p->refs = 2;
	*a = p->ends[0];
	*b = p->ends[1];
	return (0);
}

static t_object	*chan_detach(t_chanpair *p, uint32_t side, t_msgq *out,
		bool *last)
{
	uint64_t	fl;
	t_object	*peer;

	fl = spin_lock_irqsave(&p->lock);
	__atomic_store_n(&p->closed[side], 1, __ATOMIC_RELEASE);
	*out = p->q[side];
	memset(&p->q[side], 0, sizeof(t_msgq));
	p->ends[side] = NULL;
	peer = NULL;
	if (obj_tryref(p->ends[side ^ 1]))
		peer = p->ends[side ^ 1];
	p->refs--;
	*last = (p->refs == 0);
	spin_unlock_irqrestore(&p->lock, fl);
	return (peer);
}

void	chan_destroy(t_object *o)
{
	t_chanend	*e;
	t_chanpair	*p;
	t_msgq		local;
	t_object	*peer;
	bool		last;

	e = o->impl;
	p = e->pair;
	peer = chan_detach(p, e->side, &local, &last);
	ipc_bury(&local);
	if (peer)
	{
		obj_signal(peer);
		obj_unref(peer);
	}
	if (last)
		kfree(p);
	ipc_reap();
}

bool	chan_signaled(t_object *o)
{
	t_chanend	*e;
	t_chanpair	*p;

	e = o->impl;
	p = e->pair;
	if (__atomic_load_n(&p->q[e->side].count, __ATOMIC_ACQUIRE) > 0)
		return (true);
	return (__atomic_load_n(&p->closed[e->side ^ 1], __ATOMIC_ACQUIRE) != 0);
}
