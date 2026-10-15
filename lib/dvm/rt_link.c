#include "rt_int.h"

static int	rt_link_parents(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	const t_dex	*d;
	int			r;

	d = &vm->classes->dex;
	if (dc->superclass_idx == DEX_NO_INDEX)
		return (rt_verr(vm, c));
	r = dvm_class(vm, rt_dtype(d, dc->superclass_idx), &c->super);
	if (r != 0)
		return (r);
	if (c->super->is_array
		|| (c->super->access & (ACC_FINAL | ACC_INTERFACE)))
		return (rt_verr(vm, c));
	c->words = c->super->words;
	c->pay_off = c->super->pay_end;
	c->pay_end = c->super->pay_end;
	return (0);
}

static int	rt_link_ifaces(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	uint32_t	i;
	int			r;

	if (dc->interfaces.n > DIFACES_MAX)
		return (rt_verr(vm, c));
	c->ifaces = calloc(dc->interfaces.n + 1, sizeof(t_dclass *));
	if (!c->ifaces)
		return (E_NOMEM);
	i = 0;
	while (i < dc->interfaces.n)
	{
		r = dvm_class(vm, rt_dtype(&vm->classes->dex,
					dex_list_at(&dc->interfaces, i)), &c->ifaces[i]);
		if (r != 0)
			return (r);
		if (!(c->ifaces[i]->access & ACC_INTERFACE))
			return (rt_verr(vm, c));
		i++;
		c->nifaces = i;
	}
	return (0);
}

static int	rt_link_steps(t_dvm *vm, const t_dexclass *dc, t_dclass *c)
{
	int	r;

	c->access = dc->access;
	r = rt_link_parents(vm, dc, c);
	if (r == 0)
		r = rt_link_ifaces(vm, dc, c);
	if (r == 0)
		r = rt_members(vm, dc, c);
	if (r == 0)
		r = rt_verify(vm, c);
	return (r);
}

static int	rt_link_def(t_dvm *vm, uint32_t def, t_dclass **out)
{
	t_dexclass	dc;
	t_dclass	*c;
	int			r;

	*out = NULL;
	if (dex_class(&vm->classes->dex, def, &dc) != 0)
		return (E_INVAL);
	c = calloc(1, sizeof(*c));
	if (c)
		c->desc = rt_dup(rt_dtype(&vm->classes->dex, dc.class_idx));
	if (!c || !c->desc)
	{
		rt_class_free(c);
		return (E_NOMEM);
	}
	r = rt_link_steps(vm, &dc, c);
	if (r == 0 && vm->classes->n >= vm->classes->max)
		r = dvm_throw(vm, DX_NOCLASS, c->desc);
	if (r != 0)
		rt_class_free(c);
	if (r == 0)
		*out = c;
	return (r);
}

int	rt_link(t_dvm *vm, const char *desc, t_dclass **out)
{
	struct s_dclasses	*cs;
	int					def;
	int					r;

	cs = vm->classes;
	def = E_NOENT;
	if (cs->has_dex)
		def = dex_find_class(&cs->dex, desc);
	if (def < 0 || cs->def_state[def] != 0 || cs->depth >= DLINK_DEPTH)
		return (dvm_throw(vm, DX_NOCLASS, desc));
	cs->def_state[def] = 1;
	cs->depth++;
	r = rt_link_def(vm, (uint32_t)def, out);
	cs->depth--;
	cs->def_state[def] = 2 * (r != E_NOMEM);
	if (r != 0)
		return (r);
	cs->tab[cs->n++] = *out;
	return (rt_clinit(vm, *out));
}
