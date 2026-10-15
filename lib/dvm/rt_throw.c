#include "rt_int.h"

int	rt_exc_new(t_dvm *vm, t_dclass *c, const char *msg, t_dref *out)
{
	t_dref	s;
	int		r;

	s = DVM_NULL;
	r = 0;
	if (msg)
		r = dvm_string_utf8_new(vm, msg, &s);
	if (r == E_INVAL)
		r = 0;
	if (r != 0)
		return (r);
	dvm_pin(vm, s);
	r = dvm_new(vm, c, out);
	dvm_unpin(vm, s);
	if (r != 0)
		return (r);
	rt_words(rt_obj(vm, *out, DOK_OBJECT))[0] = s;
	return (0);
}

static void	rt_join(char *buf, const char *desc, const char *msg)
{
	size_t	n;
	size_t	k;
	size_t	j;

	n = 0;
	while (desc && desc[n] && n < DDESC_MAX)
	{
		buf[n] = desc[n];
		n++;
	}
	k = 0;
	while (msg && msg[k] && k < DMSG_MAX)
		k++;
	while (k > 0 && ((uint8_t)msg[k] & 0xc0) == 0x80)
		k--;
	if (k > 0)
	{
		buf[n++] = ':';
		buf[n++] = ' ';
	}
	j = 0;
	while (j < k)
		buf[n++] = msg[j++];
	buf[n] = '\0';
}

int	dvm_throw(t_dvm *vm, const char *desc, const char *msg)
{
	char		buf[DJOIN_MAX];
	t_dclass	*c;
	t_dref		exc;
	int			r;

	if (!vm)
		return (E_INVAL);
	c = rt_find(vm, desc);
	if (!c || (c->access & (ACC_ABSTRACT | ACC_INTERFACE))
		|| !rt_assignable(c, vm->classes->core[DCORE_THROWABLE]))
	{
		c = vm->classes->core[DCORE_THROWABLE];
		rt_join(buf, desc, msg);
		msg = buf;
	}
	r = rt_exc_new(vm, c, msg, &exc);
	if (r != 0)
		return (r);
	vm->pending = exc;
	return (DVM_THROWN);
}

int	dvm_throwable_message(t_dvm *vm, t_dref exc, t_dref *msg)
{
	t_dobj	*o;

	o = rt_obj(vm, exc, DOK_OBJECT);
	if (!o || !msg)
		return (E_INVAL);
	if (!rt_assignable(o->cls, vm->classes->core[DCORE_THROWABLE]))
		return (E_INVAL);
	*msg = rt_words(o)[0];
	return (0);
}

int	dvm_throwable_set_message(t_dvm *vm, t_dref exc, t_dref msg)
{
	t_dobj	*o;

	o = rt_obj(vm, exc, DOK_OBJECT);
	if (!o || o->len < 1)
		return (E_INVAL);
	if (!rt_assignable(o->cls, vm->classes->core[DCORE_THROWABLE]))
		return (E_INVAL);
	if (msg != DVM_NULL && !rt_obj(vm, msg, DOK_STRING))
		return (E_INVAL);
	rt_words(o)[0] = msg;
	return (0);
}
