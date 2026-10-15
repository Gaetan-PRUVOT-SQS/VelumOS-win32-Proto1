#include "obj_int.h"
#include "velum/heap.h"
#include "velum/libk.h"
#include "velum/util.h"

static int	recv_out(t_uptr ucr, uint64_t len, uint32_t nh)
{
	if (copy_to_user(ucr + offsetof(t_chanrecv, out_len), &len,
			sizeof(len)) < 0)
		return (E_FAULT);
	if (copy_to_user(ucr + offsetof(t_chanrecv, out_nhandles), &nh,
			sizeof(nh)) < 0)
		return (E_FAULT);
	return (0);
}

static int	recv_copy(t_uptr ucr, const t_chanrecv *cr, t_ipcmsg *m,
		const t_xfer *x)
{
	if (m->len && copy_to_user(cr->buf, m->data, m->len) < 0)
		return (E_FAULT);
	if (x->n && copy_to_user(cr->handles, x->hs, x->n * sizeof(t_handle)) < 0)
		return (E_FAULT);
	return (recv_out(ucr, m->len, m->nh));
}

static int	recv_deliver(t_object *end, t_uptr ucr, const t_chanrecv *cr,
		t_ipcmsg *m)
{
	t_xfer	x;
	t_htab	*ht;
	int		rc;

	ht = ht_of(sys_proc());
	memset(&x, 0, sizeof(x));
	x.n = m->nh;
	rc = htab_reserve(ht, &x);
	if (rc == 0)
	{
		rc = recv_copy(ucr, cr, m, &x);
		if (rc < 0)
			htab_unreserve(ht, &x);
	}
	if (rc < 0)
	{
		chan_unread(end, m);
		return (rc);
	}
	memcpy(x.objs, m->objs, sizeof(x.objs));
	memcpy(x.rights, m->rights, sizeof(x.rights));
	m->nh = 0;
	msg_free(m);
	return (htab_commit(ht, &x));
}

int64_t	sys_chan_recv(const t_sysargs *a)
{
	t_chanrecv	cr;
	t_chanrd	rd;
	t_hget		g;
	int			rc;

	if (copy_from_user(&cr, a->a[1], sizeof(cr)) < 0)
		return (E_FAULT);
	if (cr.reserved || cr.reserved2)
		return (E_INVAL);
	rc = sys_handle(a->a[0], OBJ_CHANNEL, HR_READ, &g);
	if (rc < 0)
		return (rc);
	memset(&rd, 0, sizeof(rd));
	rd.buf_len = cr.buf_len;
	rd.hmax = (uint32_t)min_u64(cr.handles_max, IPC_HANDLES_MAX);
	rc = chan_read_wait(g.obj, &rd, wait_deadline(cr.timeout_ns));
	if (rc == E_OVERFLOW && recv_out(a->a[1], rd.need_len, rd.need_nh) < 0)
		rc = E_FAULT;
	if (rc == 0)
		rc = recv_deliver(g.obj, a->a[1], &cr, rd.msg);
	obj_unref(g.obj);
	return (rc);
}
