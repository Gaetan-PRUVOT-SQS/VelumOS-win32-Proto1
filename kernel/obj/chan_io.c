#include "obj_int.h"

int	chan_write(t_object *end, t_ipcmsg *m)
{
	t_chanend	*e;
	t_chanpair	*p;
	t_object	*peer;
	uint64_t	fl;
	int			rc;

	e = end->impl;
	p = e->pair;
	peer = NULL;
	fl = spin_lock_irqsave(&p->lock);
	rc = E_PIPE;
	if (!p->closed[e->side ^ 1])
		rc = msgq_push(&p->q[e->side ^ 1], m, IPC_QUEUE_MAX);
	if (rc == 0 && obj_tryref(p->ends[e->side ^ 1]))
		peer = p->ends[e->side ^ 1];
	spin_unlock_irqrestore(&p->lock, fl);
	if (peer)
	{
		obj_signal(peer);
		obj_unref(peer);
	}
	return (rc);
}

int	chan_read_try(t_object *end, t_chanrd *rd)
{
	t_chanend	*e;
	t_chanpair	*p;
	t_ipcmsg	*m;
	uint64_t	fl;
	int			rc;

	e = end->impl;
	p = e->pair;
	fl = spin_lock_irqsave(&p->lock);
	m = p->q[e->side].head;
	rc = 0;
	if (!m && p->closed[e->side ^ 1])
		rc = E_PIPE;
	else if (!m)
		rc = E_AGAIN;
	else if (m->len > rd->buf_len || m->nh > rd->hmax)
	{
		rd->need_len = m->len;
		rd->need_nh = m->nh;
		rc = E_OVERFLOW;
	}
	else
		rd->msg = msgq_pop(&p->q[e->side]);
	spin_unlock_irqrestore(&p->lock, fl);
	return (rc);
}

void	chan_unread(t_object *end, t_ipcmsg *m)
{
	t_chanend	*e;
	uint64_t	fl;

	e = end->impl;
	fl = spin_lock_irqsave(&e->pair->lock);
	msgq_push_front(&e->pair->q[e->side], m);
	spin_unlock_irqrestore(&e->pair->lock, fl);
}

int	chan_read_wait(t_object *end, t_chanrd *rd, uint64_t deadline)
{
	int	rc;
	int	wrc;

	rc = chan_read_try(end, rd);
	wrc = 0;
	while (rc == E_AGAIN && wrc == 0)
	{
		wrc = wait_for(end, deadline);
		rc = chan_read_try(end, rd);
	}
	if (rc == E_AGAIN)
		return (wrc);
	return (rc);
}

bool	chan_same_pair(t_object *end, t_object *o)
{
	t_chanend	*a;
	t_chanend	*b;

	if (!o || o->type != OBJ_CHANNEL || o->ops->destroy != chan_destroy)
		return (false);
	a = end->impl;
	b = o->impl;
	return (a->pair == b->pair);
}
