#include "harness.h"
#include "d08.h"

static void	d08_builtin(t_dbuiltin *b, const char *desc, const char *super,
		uint32_t payload)
{
	b->desc = desc;
	b->super_desc = super;
	b->iface_desc = NULL;
	b->access = ACC_PUBLIC;
	b->payload_bytes = payload;
}

static void	integrees_et_charge_utile(void)
{
	t_dvm		*vm;
	t_dbuiltin	b[2];
	t_dclass	*c[2];
	t_dref		o;
	uint8_t		*p;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	d08_builtin(&b[0], "Lv/Vue;", "Ljava/lang/Object;", 24);
	d08_builtin(&b[1], "Lv/Texte;", "Lv/Vue;", 5);
	h_eq_i64("dvm_builtins", dvm_builtins(vm, b, 2), 0);
	h_eq_i64("doublon", dvm_builtins(vm, b, 1), E_EXIST);
	h_eq_i64("Vue", dvm_class(vm, "Lv/Vue;", &c[0]), 0);
	h_eq_i64("Texte", dvm_class(vm, "Lv/Texte;", &c[1]), 0);
	h_eq_i64("dvm_new", dvm_new(vm, c[1], &o), 0);
	h_eq_i64("instance de la mere", dvm_is_instance(vm, o, c[0]), 1);
	p = dvm_payload(vm, o, c[0]);
	h_true(p != NULL && p[23] == 0, "charge heritee a zero");
	p[23] = 7;
	h_true(dvm_payload(vm, o, c[1]) == p + 24, "charge propre a la suite");
	((uint8_t *)dvm_payload(vm, o, c[1]))[4] = 9;
	h_eq_i64("dvm_new mere", dvm_new(vm, c[0], &o), 0);
	h_true(dvm_payload(vm, o, c[1]) == NULL, "charge d'une fille refusee");
	h_true(dvm_payload(vm, o, dvm_class_of(vm, vm->heap != NULL)) == NULL,
		"classe sans charge");
	dvm_destroy(vm);
}

static void	integrees_refusees(void)
{
	t_dvm		*vm;
	t_dbuiltin	b;
	t_dclass	*c;
	t_dref		o;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	d08_builtin(&b, "Lv/A;", "Lv/Absente;", 0);
	h_eq_i64("mere inconnue", dvm_builtins(vm, &b, 1), E_NOENT);
	d08_builtin(&b, "Lv/A;", "Ljava/lang/String;", 0);
	h_eq_i64("mere finale", dvm_builtins(vm, &b, 1), E_INVAL);
	d08_builtin(&b, "[Lv/A;", "Ljava/lang/Object;", 0);
	h_eq_i64("descripteur de tableau", dvm_builtins(vm, &b, 1), E_INVAL);
	d08_builtin(&b, "Lv/A;", "Ljava/lang/Object;", 0x7fffffff);
	h_eq_i64("charge demesuree", dvm_builtins(vm, &b, 1), E_INVAL);
	d08_builtin(&b, "Lv/A;", "Ljava/lang/Object;", 0);
	b.iface_desc = "Ljava/lang/String;";
	h_eq_i64("interface qui n'en est pas une", dvm_builtins(vm, &b, 1),
		E_INVAL);
	b.iface_desc = "Ljava/lang/CharSequence;";
	b.access = ACC_PUBLIC | ACC_ABSTRACT;
	h_eq_i64("abstraite", dvm_builtins(vm, &b, 1), 0);
	h_eq_i64("classe", dvm_class(vm, "Lv/A;", &c), 0);
	h_eq_i64("abstraite non instanciable", dvm_new(vm, c, &o), E_INVAL);
	dvm_destroy(vm);
}

static void	plafond_de_classes(void)
{
	t_dvm		*vm;
	t_dlimits	lim;
	t_dbuiltin	b;
	t_dref		a;

	lim.heap_bytes = 0;
	lim.stack_words = 16;
	lim.depth_max = 0;
	lim.classes_max = 6;
	lim.budget = 0;
	h_eq_i64("dvm_create", dvm_create(&vm, &lim), 0);
	d08_builtin(&b, "Lv/A;", "Ljava/lang/Object;", 0);
	h_eq_i64("sixieme classe", dvm_builtins(vm, &b, 1), 0);
	d08_builtin(&b, "Lv/B;", "Ljava/lang/Object;", 0);
	h_eq_i64("septieme refusee", dvm_builtins(vm, &b, 1), E_RANGE);
	h_eq_i64("classe de tableau refusee", dvm_array_new(vm, "[I", 1, &a),
		E_RANGE);
	dvm_destroy(vm);
	lim.classes_max = 3;
	h_eq_i64("plafond sous le noyau", dvm_create(&vm, &lim), E_INVAL);
	h_true(vm == NULL, "rien de rendu");
}

int	main(void)
{
	h_begin("d08/classes");
	h_run("integrees_et_charge_utile", integrees_et_charge_utile);
	h_run("integrees_refusees", integrees_refusees);
	h_run("plafond_de_classes", plafond_de_classes);
	return (h_end());
}
