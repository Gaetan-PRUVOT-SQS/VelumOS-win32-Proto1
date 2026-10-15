#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"

static int	send_build(const t_chansend *cs, t_xfer *x, t_ipcmsg **out)
{
	t_ipcmsg	*m;
	int			rc;

	if ((cs->reserved & ~(uint32_t)CHAN_SEND_MOVE) || cs->len > IPC_MSG_MAX
		|| cs->nhandles > IPC_HANDLES_MAX)
		return (E_INVAL);
	memset(x, 0, sizeof(t_xfer));
	x->n = cs->nhandles;
	x->move = cs->reserved & CHAN_SEND_MOVE;
	if (x->n && copy_from_user(x->hs, cs->handles, x->n * sizeof(t_handle)))
		return (E_FAULT);
	m = msg_alloc((uint32_t)cs->len);
	if (!m)
		return (E_NOMEM);
	rc = msg_charge(sys_proc(), m);
	if (rc == 0 && cs->len && copy_from_user(m->data, cs->msg, cs->len) < 0)
		rc = E_FAULT;
	if (rc < 0)
		msg_free(m);
	if (rc == 0)
		*out = m;
	return (rc);
}

static int	send_self_check(t_object *end, const t_xfer *x)
{
	uint32_t	i;

	i = 0;
	while (i < x->n)
	{
		if (x->objs[i] == end || chan_same_pair(end, x->objs[i]))
			return (E_INVAL);
		i++;
	}
	return (0);
}

static void	send_fill(t_ipcmsg *m, const t_xfer *x)
{
	uint32_t	i;

	i = 0;
	while (i < x->n)
	{
		m->objs[i] = x->objs[i];
		m->rights[i] = x->rights[i];
		i++;
	}
	m->nh = x->n;
}

static int	send_commit(t_object *end, t_xfer *x, t_ipcmsg *m)
{
	t_htab	*ht;
	int		rc;

	ht = ht_of(sys_proc());
	rc = htab_take(ht, x);
	if (rc < 0)
		return (rc);
	rc = send_self_check(end, x);
	if (rc == 0)
	{
		send_fill(m, x);
		rc = chan_write(end, m);
	}
	if (rc < 0)
		m->nh = 0;
	htab_take_end(ht, x, rc == 0);
	if (rc < 0)
		xfer_drop(x);
	return (rc);
}

int64_t	sys_chan_send(const t_sysargs *a)
{
	t_chansend	cs;
	t_xfer		x;
	t_hget		g;
	t_ipcmsg	*m;
	int			rc;

	if (copy_from_user(&cs, a->a[1], sizeof(cs)) < 0)
		return (E_FAULT);
	rc = sys_handle(a->a[0], OBJ_CHANNEL, HR_WRITE, &g);
	if (rc < 0)
		return (rc);
	rc = send_build(&cs, &x, &m);
	if (rc == 0)
	{
		rc = send_commit(g.obj, &x, m);
		if (rc < 0)
			msg_free(m);
	}
	obj_unref(g.obj);
	return (rc);
}
