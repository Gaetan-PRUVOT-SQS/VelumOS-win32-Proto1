#include "obj_int.h"
#include "velum/heap.h"

static void	evt_destroy(t_object *o)
{
	kfree(o->impl);
}

static const t_objops	g_evt_ops = {"event", evt_destroy, sig_signaled};

int	evt_create(uint32_t manual, uint32_t initial, t_object **out)
{
	t_sigstate	*s;

	if (manual > 1 || initial > 1 || !out)
		return (E_INVAL);
	s = kmalloc_tag(sizeof(t_sigstate), HEAP_OBJECT);
	if (!s)
		return (E_NOMEM);
	s->state = initial;
	s->manual = manual;
	*out = obj_create(OBJ_EVENT, &g_evt_ops, s);
	if (*out)
		return (0);
	kfree(s);
	return (E_NOMEM);
}

int	evt_op(t_object *o, uint32_t op)
{
	t_sigstate	*s;

	if (!o || o->type != OBJ_EVENT || !sig_is(o))
		return (E_INVAL);
	s = o->impl;
	if (op == EV_SET)
		sig_set(o, 1);
	else if (op == EV_RESET)
		sig_set(o, 0);
	else if (op == EV_PULSE)
	{
		sig_set(o, 0);
		wreg_pulse(o, s->manual != 0);
	}
	else
		return (E_INVAL);
	return (0);
}
