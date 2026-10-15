#include "harness.h"
#include "d08.h"

static void	creation_chaque_malloc_en_echec(void)
{
	t_dvm		*vm;
	uint32_t	k;
	int			r;

	k = 0;
	r = E_NOMEM;
	while (r != 0 && k < 64)
	{
		d08_fail_after(k);
		r = dvm_create(&vm, NULL);
		d08_fail_off();
		if (r != 0)
			h_true(r == E_NOMEM && vm == NULL, "echec propre");
		k++;
	}
	h_eq_i64("finit par reussir", r, 0);
	h_true(k > 8, "plusieurs allocations traversees");
	dvm_destroy(vm);
}

static int	d08_scenario(t_dvm *vm)
{
	t_dbuiltin	b;
	t_dref		o;
	int			r;

	b.desc = "Lv/Echec;";
	b.super_desc = "Ljava/lang/Object;";
	b.iface_desc = NULL;
	b.access = ACC_PUBLIC;
	b.payload_bytes = 16;
	r = dvm_builtins(vm, &b, 1);
	if (r == 0 || r == E_EXIST)
		r = dvm_string_utf8_new(vm, "abc", &o);
	if (r == 0)
		r = dvm_array_new(vm, "[[Ljava/lang/String;", 4, &o);
	if (r == 0)
		r = dvm_throw(vm, "Lx/Absente;", "message");
	if (r == DVM_THROWN)
		r = 0;
	return (r);
}

static void	operations_chaque_malloc_en_echec(void)
{
	t_dvm		*vm;
	uint32_t	k;
	int			r;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	k = 0;
	r = E_NOMEM;
	while (r != 0 && k < 64)
	{
		d08_fail_after(k);
		r = d08_scenario(vm);
		d08_fail_off();
		if (r != 0)
			h_eq_i64("echec propre", r, E_NOMEM);
		dvm_gc(vm);
		k++;
	}
	h_eq_i64("finit par reussir", r, 0);
	h_true(k > 4, "plusieurs allocations traversees");
	h_eq_i64("machine utilisable", d08_scenario(vm), 0);
	dvm_destroy(vm);
}

int	main(void)
{
	h_begin("d08/malloc");
	h_run("creation_chaque_malloc_en_echec", creation_chaque_malloc_en_echec);
	h_run("operations_chaque_malloc_en_echec",
		operations_chaque_malloc_en_echec);
	return (h_end());
}
