#include "rt_int.h"

static int	rt_same(const t_dmethod *m, const t_dname *nm)
{
	if (!m->name || !m->sig)
		return (0);
	return (strcmp(m->name, nm->name) == 0 && strcmp(m->sig, nm->sig) == 0);
}

t_dmethod	*rt_method_own(const t_dvm *vm, const t_dclass *c,
		const t_dname *nm)
{
	const struct s_dclasses	*cs;
	uint32_t				i;
	uint32_t				k;

	i = 0;
	while (i < c->nmethods)
	{
		if (rt_same(&c->methods[i], nm))
			return (&c->methods[i]);
		i++;
	}
	cs = vm->classes;
	k = 0;
	while (k < cs->nnat)
	{
		i = 0;
		while (i < cs->natn[k])
		{
			if (cs->natm[k][i].cls == c && rt_same(&cs->natm[k][i], nm))
				return (&cs->natm[k][i]);
			i++;
		}
		k++;
	}
	return (NULL);
}

t_dmethod	*rt_method_find(const t_dvm *vm, const t_dclass *c,
		const t_dname *nm)
{
	const t_dclass	*k;
	t_dmethod		*m;
	uint32_t		i;

	k = c;
	m = NULL;
	while (k && !m)
	{
		m = rt_method_own(vm, k, nm);
		k = k->super;
	}
	k = c;
	while (k && !m)
	{
		i = 0;
		while (i < k->nifaces && !m)
			m = rt_method_find(vm, k->ifaces[i++], nm);
		k = k->super;
	}
	return (m);
}

int	dvm_method(t_dvm *vm, t_dclass *c, const t_dname *name,
		const t_dmethod **out)
{
	if (!vm || !c || !name || !name->name || !name->sig || !out)
		return (E_INVAL);
	*out = rt_method_find(vm, c, name);
	if (!*out)
		return (E_NOENT);
	return (0);
}

int	rt_clinit(t_dvm *vm, t_dclass *c)
{
	t_dname			nm;
	const t_dmethod	*m;
	uint64_t		ret;

	nm.name = "<clinit>";
	nm.sig = "()V";
	m = rt_method_own(vm, c, &nm);
	if (!m || c->init)
		return (0);
	c->init = 1;
	ret = 0;
	return (dvm_call(vm, m, NULL, &ret));
}
