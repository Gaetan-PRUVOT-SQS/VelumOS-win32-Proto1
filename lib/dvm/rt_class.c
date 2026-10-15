#include "rt_int.h"

t_dclass	*rt_find(const t_dvm *vm, const char *desc)
{
	uint32_t	i;

	i = 0;
	while (desc && i < vm->classes->n)
	{
		if (strcmp(vm->classes->tab[i]->desc, desc) == 0)
			return (vm->classes->tab[i]);
		i++;
	}
	return (NULL);
}

char	*rt_dup(const char *s)
{
	char	*d;
	size_t	n;

	if (!s)
		return (NULL);
	n = strlen(s) + 1;
	d = calloc(1, n);
	while (d && n > 0)
	{
		n--;
		d[n] = s[n];
	}
	return (d);
}

static int	rt_parents(const t_dvm *vm, const t_dbuiltin *b, t_dclass *c)
{
	c->super = rt_find(vm, b->super_desc);
	c->iface = rt_find(vm, b->iface_desc);
	if ((b->super_desc && !c->super) || (b->iface_desc && !c->iface))
		return (E_NOENT);
	if (c->super && (c->super->access & (ACC_INTERFACE | ACC_FINAL)))
		return (E_INVAL);
	if (c->iface && !(c->iface->access & ACC_INTERFACE))
		return (E_INVAL);
	c->access = b->access;
	c->ifaces = &c->iface;
	c->nifaces = (c->iface != NULL);
	if (c->super)
	{
		c->words = c->super->words;
		c->pay_off = c->super->pay_end;
	}
	c->pay_end = c->pay_off + ((b->payload_bytes + 7u) & ~7u);
	return (0);
}

int	rt_define(t_dvm *vm, const t_dbuiltin *b, t_dclass **out)
{
	t_dclass	*c;
	int			r;

	if (!b || !b->desc || b->payload_bytes > DPAYLOAD_MAX
		|| strlen(b->desc) > DDESC_MAX)
		return (E_INVAL);
	if (rt_find(vm, b->desc))
		return (E_EXIST);
	if (vm->classes->n >= vm->classes->max)
		return (E_RANGE);
	c = calloc(1, sizeof(*c));
	if (!c)
		return (E_NOMEM);
	r = rt_parents(vm, b, c);
	if (r == 0)
		c->desc = rt_dup(b->desc);
	if (r == 0 && !c->desc)
		r = E_NOMEM;
	if (r != 0)
		free(c);
	if (r != 0)
		return (r);
	vm->classes->tab[vm->classes->n++] = c;
	*out = c;
	return (0);
}

int	dvm_builtins(t_dvm *vm, const t_dbuiltin *tab, uint32_t n)
{
	t_dclass	*c;
	uint32_t	i;
	int			r;

	if (!vm || (!tab && n))
		return (E_INVAL);
	i = 0;
	while (i < n)
	{
		if (!tab[i].desc || tab[i].desc[0] != 'L')
			return (E_INVAL);
		r = rt_define(vm, &tab[i], &c);
		if (r != 0)
			return (r);
		i++;
	}
	rt_oom_retype(vm);
	return (0);
}
