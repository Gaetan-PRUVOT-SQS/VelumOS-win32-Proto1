#include "rt_int.h"

void	rt_mark(const t_dvm *vm, uint32_t value)
{
	t_dobj	*o;

	o = rt_obj(vm, value, DOK_ANY);
	if (o && o->mark == 0)
		o->mark = 1;
}

static void	rt_scan(const t_dvm *vm, t_dobj *o)
{
	const uint32_t	*w;
	uint32_t		i;

	o->mark = 2;
	w = NULL;
	if (o->kind == DOK_OBJECT)
		w = rt_words(o);
	if (o->kind == DOK_ARRAY && o->cls->is_ref)
		w = o->data;
	i = 0;
	while (w && i < o->len)
		rt_mark(vm, w[i++]);
}

static void	rt_roots(const t_dvm *vm)
{
	uint32_t	i;

	i = 0;
	while (vm->stack && i < vm->sp && i < vm->stack_words)
		rt_mark(vm, vm->stack[i++]);
	i = 0;
	while (i < vm->nlocals && i < DVM_LOCALS_MAX)
		rt_mark(vm, vm->locals[i++]);
	rt_mark(vm, vm->pending);
	rt_mark(vm, (t_dref)vm->result);
	rt_mark(vm, vm->heap->oom);
	rt_roots_classes(vm);
	i = 1;
	while (i < vm->heap->cap)
	{
		if (vm->heap->tab[i].kind != DOK_FREE && vm->heap->tab[i].pins)
			vm->heap->tab[i].mark = 1;
		i++;
	}
}

static uint32_t	rt_pass(const t_dvm *vm)
{
	uint32_t	i;
	uint32_t	n;

	i = 1;
	n = 0;
	while (i < vm->heap->cap)
	{
		if (vm->heap->tab[i].kind != DOK_FREE && vm->heap->tab[i].mark == 1)
		{
			rt_scan(vm, &vm->heap->tab[i]);
			n++;
		}
		i++;
	}
	return (n);
}

void	dvm_gc(t_dvm *vm)
{
	uint32_t	i;

	if (!vm || !vm->heap)
		return ;
	rt_roots(vm);
	i = 1;
	while (i != 0)
		i = rt_pass(vm);
	i = 1;
	while (i < vm->heap->cap)
	{
		if (vm->heap->tab[i].kind != DOK_FREE && vm->heap->tab[i].mark == 0)
			rt_release(vm->heap, i);
		else
			vm->heap->tab[i].mark = 0;
		i++;
	}
	vm->heap->st.collections++;
}
