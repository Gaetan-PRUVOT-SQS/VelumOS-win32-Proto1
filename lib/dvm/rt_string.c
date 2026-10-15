#include "rt_int.h"

static int	rt_str_alloc(t_dvm *vm, uint32_t n, t_dref *out)
{
	t_dnewobj	rq;

	if (n > DSTR_MAX)
		return (E_RANGE);
	rq.cls = vm->classes->core[DCORE_STRING];
	rq.len = n;
	rq.bytes = (uint64_t)n * 2u;
	rq.kind = DOK_STRING;
	return (rt_alloc(vm, &rq, out));
}

int	dvm_string_utf16_new(t_dvm *vm, t_dstr16 s, t_dref *out)
{
	uint16_t	*d;
	uint32_t	i;
	int			r;

	if (!vm || !out || (!s.p && s.n))
		return (E_INVAL);
	r = rt_str_alloc(vm, s.n, out);
	if (r != 0)
		return (r);
	d = rt_obj(vm, *out, DOK_STRING)->data;
	i = 0;
	while (i < s.n)
	{
		d[i] = s.p[i];
		i++;
	}
	return (0);
}

int	dvm_string_utf8_new(t_dvm *vm, const char *utf8, t_dref *out)
{
	int	n;
	int	r;

	if (!vm || !utf8 || !out)
		return (E_INVAL);
	*out = DVM_NULL;
	n = rt_utf8_to16(utf8, NULL);
	if (n < 0)
		return (n);
	r = rt_str_alloc(vm, (uint32_t)n, out);
	if (r != 0)
		return (r);
	rt_utf8_to16(utf8, rt_obj(vm, *out, DOK_STRING)->data);
	return (0);
}

int	dvm_string_get(t_dvm *vm, t_dref ref, t_dstr16 *out)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_STRING);
	if (!o || !out)
		return (E_INVAL);
	out->p = o->data;
	out->n = o->len;
	return (0);
}

int	dvm_string_utf8(t_dvm *vm, t_dref ref, t_text out)
{
	t_dstr16	s;

	if (dvm_string_get(vm, ref, &s) != 0)
		return (E_INVAL);
	return (rt_utf16_to8(s, out));
}
