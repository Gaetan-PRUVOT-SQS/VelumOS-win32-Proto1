#include "rt_int.h"

static int	rt_res_miss(t_dvm *vm, const t_dclass *c, const t_dname *nm)
{
	char	buf[DJOIN_MAX];
	size_t	n;

	n = rt_cat(buf, 0, c->desc);
	n = rt_cat(buf, n, "->");
	n = rt_cat(buf, n, nm->name);
	rt_cat(buf, n, nm->sig);
	return (dvm_throw(vm, "Ljava/lang/NoSuchMethodError;", buf));
}

static int	rt_res_self(t_dvm *vm, const t_dinvoke *inv, t_dclass **c)
{
	t_dobj	*o;

	if (inv->kind == DIK_STATIC)
		return (0);
	if (inv->self == DVM_NULL)
		return (dvm_throw(vm, DX_NPE, NULL));
	o = rt_obj(vm, inv->self, DOK_ANY);
	if (!o || !rt_assignable(o->cls, *c))
		return (dvm_throw(vm, DX_CCE, NULL));
	if (inv->kind == DIK_VIRTUAL || inv->kind == DIK_INTERFACE)
		*c = o->cls;
	return (0);
}

static int	rt_res_pick(t_dvm *vm, t_dinvoke *inv, const t_dclass *start,
		const t_dname *nm)
{
	const t_dmethod	*m;

	if (inv->kind == DIK_DIRECT)
		m = rt_method_own(vm, start, nm);
	else
		m = rt_method_find(vm, start, nm);
	if (!m)
		return (rt_res_miss(vm, start, nm));
	if (((m->access & ACC_STATIC) != 0) != (inv->kind == DIK_STATIC))
		return (dvm_throw(vm, "Ljava/lang/IncompatibleClassChangeError;",
				nm->name));
	if (!m->native && m->code_off == 0)
		return (dvm_throw(vm, "Ljava/lang/AbstractMethodError;", nm->name));
	inv->target = m;
	return (0);
}

static char	*rt_res_name(t_dvm *vm, uint32_t idx, t_dname *nm, uint32_t *cls)
{
	const t_dex	*d;
	t_dexmethod	dm;
	t_dexproto	p;
	char		*sig;

	d = &vm->classes->dex;
	nm->name = NULL;
	if (!vm->classes->has_dex || dex_method(d, idx, &dm) != 0
		|| dex_proto(d, dm.proto_idx, &p) != 0)
		return (NULL);
	sig = rt_sig_build(d, &p);
	nm->name = rt_dstr(d, dm.name_idx);
	nm->sig = sig;
	*cls = dm.class_idx;
	return (sig);
}

int	dvmrt_resolve(t_dvm *vm, const t_dmethod *from, t_dinvoke *inv)
{
	t_dname		nm;
	t_dclass	*c;
	char		*sig;
	uint32_t	cls;
	int			r;

	if (!vm || !inv || inv->kind > DIK_INTERFACE)
		return (E_INVAL);
	inv->target = NULL;
	sig = rt_res_name(vm, inv->method_idx, &nm, &cls);
	if (!sig && nm.name)
		return (E_NOMEM);
	if (!sig || !nm.name)
		return (free(sig), E_INVAL);
	r = rt_class_idx(vm, cls, &c);
	if (r == 0)
		r = rt_res_self(vm, inv, &c);
	if (r == 0 && inv->kind == DIK_SUPER && from && from->cls
		&& from->cls->super)
		c = from->cls->super;
	if (r == 0)
		r = rt_res_pick(vm, inv, c, &nm);
	free(sig);
	return (r);
}
