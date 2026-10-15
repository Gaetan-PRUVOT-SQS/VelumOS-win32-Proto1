#include "harness.h"
#include "d08.h"

static int	d08_petite(t_dvm **vm, uint32_t heap)
{
	t_dlimits	lim;

	lim.heap_bytes = heap;
	lim.stack_words = 16;
	lim.depth_max = 0;
	lim.classes_max = 0;
	lim.budget = 0;
	return (dvm_create(vm, &lim));
}

static void	plafond_donne_outofmemory(void)
{
	t_dvm	*vm;
	t_dref	a;
	t_dref	last;
	int		r;

	h_eq_i64("dvm_create", d08_petite(&vm, 8192), 0);
	h_eq_i64("demesure", dvm_array_new(vm, "[J", 0x7fffffff, &a), DVM_THROWN);
	last = DVM_NULL;
	r = dvm_array_new(vm, "[B", 500, &a);
	while (r == 0)
	{
		last = a;
		dvm_pin(vm, last);
		r = dvm_array_new(vm, "[B", 500, &a);
	}
	h_eq_i64("plafond atteint", r, DVM_THROWN);
	h_true(last != DVM_NULL, "au moins un tableau avant le plafond");
	h_eq_i64("exception", dvm_throwable_message(vm, vm->pending, &a), 0);
	h_true(dvm_class_of(vm, a) != NULL, "message vivant");
	dvm_unpin(vm, last);
	h_eq_i64("utilisable ensuite", dvm_array_new(vm, "[B", 500, &a), 0);
	dvm_destroy(vm);
}

static void	plafond_trop_bas_pour_le_noyau(void)
{
	t_dvm	*vm;

	h_eq_i64("tas de 8 octets", d08_petite(&vm, 8), E_NOMEM);
	h_true(vm == NULL, "rien de rendu");
}

int	main(void)
{
	h_begin("d08/plafond");
	h_run("plafond_donne_outofmemory", plafond_donne_outofmemory);
	h_run("plafond_trop_bas_pour_le_noyau", plafond_trop_bas_pour_le_noyau);
	return (h_end());
}
