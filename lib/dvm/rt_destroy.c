#include "rt_int.h"

void	rt_class_free(t_dclass *c)
{
	uint32_t	i;

	if (!c)
		return ;
	i = 0;
	while (i < c->nmethods)
	{
		if (c->starts)
			free(c->starts[i]);
		if (c->sigs)
			free(c->sigs[i]);
		i++;
	}
	if (c->ifaces != &c->iface)
		free(c->ifaces);
	free(c->methods);
	free(c->starts);
	free(c->sigs);
	free(c->fields);
	free(c->statics);
	free(c->desc);
	free(c);
}

static void	rt_classes_free(struct s_dclasses *cs)
{
	uint32_t	i;

	i = 0;
	while (cs->tab && i < cs->n)
		rt_class_free(cs->tab[i++]);
	i = 0;
	while (i < cs->nnat)
		free(cs->natm[i++]);
	free(cs->def_state);
	free(cs->interned);
	free(cs->tab);
	free(cs);
}

static void	rt_heap_free(struct s_dheap *h)
{
	uint32_t	i;

	i = 1;
	while (h->tab && i < h->cap)
	{
		free(h->tab[i].data);
		i++;
	}
	free(h->tab);
	free(h);
}

void	dvm_destroy(t_dvm *vm)
{
	if (!vm)
		return ;
	if (vm->heap)
		rt_heap_free(vm->heap);
	if (vm->classes)
		rt_classes_free(vm->classes);
	free(vm->stack);
	free(vm);
}

void	rt_roots_classes(const t_dvm *vm)
{
	const struct s_dclasses	*cs;
	uint32_t				i;
	uint32_t				k;

	cs = vm->classes;
	i = 0;
	while (i < cs->n)
	{
		rt_mark(vm, cs->tab[i]->mirror);
		k = 0;
		while (cs->tab[i]->statics && k < cs->tab[i]->nstatics)
			rt_mark(vm, cs->tab[i]->statics[k++]);
		i++;
	}
	i = 0;
	while (cs->interned && i < cs->dex.n[DEX_T_STRING])
		rt_mark(vm, cs->interned[i++]);
}
