#include "fake.h"

void	fk_chan_pair(t_process *pa, t_process *pb, t_handle *out)
{
	t_object	*a;
	t_object	*b;

	if (chan_create(&a, &b) < 0)
		return ;
	handle_alloc(pa, a, RIGHTS_CHAN, &out[0]);
	handle_alloc(pb, b, RIGHTS_CHAN, &out[1]);
	obj_unref(a);
	obj_unref(b);
}

t_handle	fk_event_in(t_process *p, uint32_t rights)
{
	t_object	*e;
	t_handle	h;

	h = HANDLE_INVALID;
	if (evt_create(1, 0, &e) < 0)
		return (h);
	handle_alloc(p, e, rights, &h);
	obj_unref(e);
	return (h);
}

int64_t	fk_send(t_handle h, const t_chansend *cs)
{
	return (fk_call(SYS_CHAN_SEND, h, fk_uptr(cs), 0));
}

int64_t	fk_recv(t_handle h, t_chanrecv *cr)
{
	return (fk_call(SYS_CHAN_RECV, h, fk_uptr(cr), 0));
}

t_object	*fk_obj(t_process *p, t_handle h)
{
	t_hget	g;

	if (handle_get(p, h, OBJ_NONE, &g) < 0)
		return (NULL);
	obj_unref(g.obj);
	return (g.obj);
}
