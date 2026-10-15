#include "harness.h"
#include "d08.h"

static void	objet_inatteignable_repris(void)
{
	t_dvm			*vm;
	t_dref			s;
	t_dheapstats	avant;
	t_dheapstats	apres;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	dvm_heap_stats(vm, &avant);
	h_eq_i64("chaine", dvm_string_utf8_new(vm, "perdue", &s), 0);
	dvm_gc(vm);
	dvm_heap_stats(vm, &apres);
	h_eq_u64("objets", apres.objects, avant.objects);
	h_eq_u64("octets", apres.bytes, avant.bytes);
	h_eq_u64("collectes", apres.collections, avant.collections + 1);
	h_eq_i64("case reutilisee", dvm_string_utf8_new(vm, "autre", &avant.bytes),
		0);
	h_eq_u64("meme indice", avant.bytes, s);
	dvm_destroy(vm);
}

static void	d08_racines(t_dvm *vm, t_dref *r)
{
	t_darrview	v;

	h_eq_i64("pile", dvm_string_utf8_new(vm, "pile", &r[0]), 0);
	vm->stack[0] = r[0];
	vm->stack[1] = 0xdeadbeefu;
	vm->sp = 2;
	h_eq_i64("local", dvm_string_utf8_new(vm, "local", &r[1]), 0);
	h_eq_i64("dvm_local", dvm_local(vm, r[1]), 0);
	h_eq_i64("tableau", dvm_array_new(vm, "[Ljava/lang/String;", 1, &r[2]), 0);
	h_eq_i64("dvm_pin", dvm_pin(vm, r[2]), 0);
	h_eq_i64("element", dvm_string_utf8_new(vm, "element", &r[3]), 0);
	h_eq_i64("vue", dvm_array_view(vm, r[2], &v), 0);
	((uint32_t *)v.data)[0] = r[3];
	h_eq_i64("exception", dvm_throw(vm, "Lx/Erreur;", NULL), DVM_THROWN);
	h_eq_i64("message", dvm_throwable_message(vm, vm->pending, &r[4]), 0);
}

static void	racines_gardees(void)
{
	t_dvm	*vm;
	t_dref	r[5];

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	d08_racines(vm, r);
	dvm_gc(vm);
	h_true(dvm_class_of(vm, r[0]) != NULL, "atteint par la pile");
	h_true(dvm_class_of(vm, r[1]) != NULL, "atteint par un local");
	h_true(dvm_class_of(vm, r[3]) != NULL, "atteint par un tableau epingle");
	h_true(dvm_class_of(vm, r[4]) != NULL, "atteint par un champ");
	vm->sp = 0;
	dvm_local_pop(vm, 9);
	dvm_unpin(vm, r[2]);
	dvm_gc(vm);
	h_true(dvm_class_of(vm, r[0]) == NULL && dvm_class_of(vm, r[3]) == NULL,
		"repris une fois les racines retirees");
	dvm_destroy(vm);
}

static void	cent_mille_allocations_sous_plafond(void)
{
	t_dvm			*vm;
	t_dlimits		lim;
	t_dref			a;
	uint32_t		i;
	t_dheapstats	st;

	lim.heap_bytes = 65536;
	lim.stack_words = 16;
	lim.depth_max = 0;
	lim.classes_max = 0;
	lim.budget = 0;
	h_eq_i64("dvm_create", dvm_create(&vm, &lim), 0);
	i = 0;
	while (i < 100000 && dvm_array_new(vm, "[I", 64, &a) == 0)
		i++;
	h_eq_u64("toutes reussies", i, 100000);
	dvm_heap_stats(vm, &st);
	h_true(st.peak_bytes <= 65536, "pic sous le plafond");
	h_true(st.collections > 100, "collectes declenchees");
	h_true(vm->heap != NULL && st.objects < 300, "pas de croissance");
	dvm_destroy(vm);
}

int	main(void)
{
	h_begin("d08/gc");
	h_run("objet_inatteignable_repris", objet_inatteignable_repris);
	h_run("racines_gardees", racines_gardees);
	h_run("cent_mille_allocations_sous_plafond",
		cent_mille_allocations_sous_plafond);
	return (h_end());
}
