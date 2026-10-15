#include "rt_int.h"

static uint8_t	rt_prim_width(char c)
{
	if (c == 'Z' || c == 'B')
		return (1);
	if (c == 'C' || c == 'S')
		return (2);
	if (c == 'I' || c == 'F')
		return (4);
	if (c == 'J' || c == 'D')
		return (8);
	return (0);
}

static int	rt_elem(t_dvm *vm, const char *e, t_dclass **cls, uint8_t *w)
{
	*cls = NULL;
	*w = rt_prim_width(e[0]);
	if (*w && e[1] == '\0')
		return (0);
	*w = 4;
	if (e[0] == 'L' || e[0] == '[')
		return (dvm_class(vm, e, cls));
	return (E_INVAL);
}

int	rt_array_class(t_dvm *vm, const char *desc, t_dclass **out)
{
	t_dbuiltin	b;
	t_dclass	*elem;
	uint8_t		width;
	int			r;

	*out = NULL;
	if (desc[0] != '[' || strlen(desc) > DDESC_MAX)
		return (E_INVAL);
	r = rt_elem(vm, desc + 1, &elem, &width);
	if (r != 0)
		return (r);
	b.desc = desc;
	b.super_desc = "Ljava/lang/Object;";
	b.iface_desc = NULL;
	b.access = ACC_PUBLIC | ACC_FINAL;
	b.payload_bytes = 0;
	r = rt_define(vm, &b, out);
	if (r != 0)
		return (r);
	(*out)->elem = elem;
	(*out)->width = width;
	(*out)->is_ref = (elem != NULL);
	(*out)->is_array = 1;
	return (0);
}

int	dvm_array_new(t_dvm *vm, const char *desc, int32_t len, t_dref *out)
{
	t_dnewobj	rq;
	int			r;

	if (!vm || !desc || !out || desc[0] != '[')
		return (E_INVAL);
	*out = DVM_NULL;
	r = dvm_class(vm, desc, &rq.cls);
	if (r != 0)
		return (r);
	if (len < 0)
		return (dvm_throw(vm, "Ljava/lang/NegativeArraySizeException;",
				NULL));
	rq.len = (uint32_t)len;
	rq.bytes = (uint64_t)rq.len * rq.cls->width;
	rq.kind = DOK_ARRAY;
	return (rt_alloc(vm, &rq, out));
}

int	dvm_array_view(t_dvm *vm, t_dref ref, t_darrview *out)
{
	t_dobj	*o;

	o = rt_obj(vm, ref, DOK_ARRAY);
	if (!o || !out)
		return (E_INVAL);
	out->data = o->data;
	out->len = o->len;
	out->width = o->cls->width;
	out->is_ref = o->cls->is_ref;
	return (0);
}
