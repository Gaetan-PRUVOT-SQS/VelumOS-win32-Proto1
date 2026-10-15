#include "rt_int.h"

static int	rt_core_one(t_dvm *vm, uint32_t slot, const char *desc,
		uint32_t access)
{
	t_dbuiltin	b;

	b.desc = desc;
	b.super_desc = "Ljava/lang/Object;";
	b.iface_desc = NULL;
	if (slot == DCORE_OBJECT || slot == DCORE_CHARSEQ)
		b.super_desc = NULL;
	if (slot == DCORE_STRING)
		b.iface_desc = "Ljava/lang/CharSequence;";
	b.access = access;
	b.payload_bytes = 0;
	return (rt_define(vm, &b, &vm->classes->core[slot]));
}

static int	rt_core(t_dvm *vm)
{
	int	r;

	r = rt_core_one(vm, DCORE_OBJECT, "Ljava/lang/Object;", ACC_PUBLIC);
	if (r == 0)
		r = rt_core_one(vm, DCORE_CLASS, "Ljava/lang/Class;",
				ACC_PUBLIC | ACC_FINAL);
	if (r == 0)
		r = rt_core_one(vm, DCORE_CHARSEQ, "Ljava/lang/CharSequence;",
				ACC_PUBLIC | ACC_INTERFACE | ACC_ABSTRACT);
	if (r == 0)
		r = rt_core_one(vm, DCORE_STRING, "Ljava/lang/String;",
				ACC_PUBLIC | ACC_FINAL);
	if (r == 0)
		r = rt_core_one(vm, DCORE_THROWABLE, "Ljava/lang/Throwable;",
				ACC_PUBLIC);
	if (r != 0)
		return (r);
	vm->classes->core[DCORE_THROWABLE]->words = 1;
	vm->classes->core[DCORE_CLASS]->pay_end = 8;
	return (rt_exc_new(vm, vm->classes->core[DCORE_THROWABLE],
			"Ljava/lang/OutOfMemoryError;", &vm->heap->oom));
}

static int	rt_limits(t_dlimits *lim)
{
	if (lim->heap_bytes == 0)
		lim->heap_bytes = DVM_HEAP_DEFAULT;
	if (lim->stack_words == 0)
		lim->stack_words = DVM_STACK_WORDS_DEFAULT;
	if (lim->depth_max == 0)
		lim->depth_max = DVM_DEPTH_DEFAULT;
	if (lim->classes_max == 0)
		lim->classes_max = DVM_CLASSES_DEFAULT;
	if (lim->classes_max < DCORE_N || lim->classes_max > DCLASSES_HARD)
		return (E_INVAL);
	if (lim->stack_words > DSTACK_HARD)
		return (E_INVAL);
	return (0);
}

static int	rt_parts(t_dvm *vm)
{
	vm->heap = calloc(1, sizeof(*vm->heap));
	vm->classes = calloc(1, sizeof(*vm->classes));
	vm->stack = calloc(vm->lim.stack_words, sizeof(uint32_t));
	if (!vm->heap || !vm->classes || !vm->stack)
		return (E_NOMEM);
	vm->stack_words = vm->lim.stack_words;
	vm->heap->limit = vm->lim.heap_bytes;
	vm->classes->max = vm->lim.classes_max;
	vm->classes->tab = calloc(vm->lim.classes_max, sizeof(t_dclass *));
	if (!vm->classes->tab)
		return (E_NOMEM);
	return (rt_core(vm));
}

int	dvm_create(t_dvm **out, const t_dlimits *lim)
{
	t_dvm	*vm;
	int		r;

	if (!out)
		return (E_INVAL);
	*out = NULL;
	vm = calloc(1, sizeof(*vm));
	if (!vm)
		return (E_NOMEM);
	if (lim)
		vm->lim = *lim;
	r = rt_limits(&vm->lim);
	if (r == 0)
		r = rt_parts(vm);
	if (r != 0)
	{
		dvm_destroy(vm);
		return (r);
	}
	*out = vm;
	return (0);
}
