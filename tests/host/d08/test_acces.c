#include "harness.h"
#include "d08.h"

static void	champs_statiques_entiers_et_longs(void)
{
	t_dvm	*vm;
	t_span	file;
	t_dfacc	a;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	a = (t_dfacc){0, DVM_NULL, 0, 0, 0, 0, 1};
	a.field_idx = (uint32_t)d08_field_idx(vm, "Ld08/Base;", "compteur");
	h_eq_i64("statique entier", dvmrt_field(vm, NULL, &a), 0);
	h_eq_u64("valeur initiale", a.val, 7);
	a.wide = 1;
	h_eq_i64("largeur fausse", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	a.field_idx = (uint32_t)d08_field_idx(vm, "Ld08/Base;", "grand");
	h_eq_i64("statique long", dvmrt_field(vm, NULL, &a), 0);
	h_eq_u64("valeur longue", a.val, 5000000000u);
	a.put = 1;
	a.val = 9;
	h_eq_i64("sput-wide", dvmrt_field(vm, NULL, &a), 0);
	a.field_idx = 0x00ffffffu;
	h_eq_i64("indice de champ hors table", dvmrt_field(vm, NULL, &a), E_INVAL);
	d08_close(vm, file);
}

static void	champs_statiques_chaines(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dfacc		a;
	t_dstr16	s;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	a = (t_dfacc){0, DVM_NULL, 0, 0, 1, 0, 1};
	a.field_idx = (uint32_t)d08_field_idx(vm, "Ld08/Base;", "nom");
	h_eq_i64("statique chaine", dvmrt_field(vm, NULL, &a), 0);
	h_eq_i64("chaine internee", dvm_string_get(vm, (t_dref)a.val, &s), 0);
	h_eq_u64("quatre lettres", s.n, 4);
	dvm_gc(vm);
	h_eq_i64("gardee par le statique", dvm_string_get(vm, (t_dref)a.val, &s),
		0);
	a.is_ref = 0;
	h_eq_i64("genre faux", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	a.is_ref = 1;
	a.put = 1;
	a.val = 424242u;
	h_eq_i64("reference hostile ecrite", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	d08_close(vm, file);
}

static void	champs_d_instance(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dclass	*c;
	t_dfacc		a;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("Fille", dvm_class(vm, "Ld08/Fille;", &c), 0);
	a = (t_dfacc){0, DVM_NULL, 0x1122334455667788u, 1, 0, 1, 0};
	a.field_idx = (uint32_t)d08_field_idx(vm, "Ld08/Base;", "y");
	h_eq_i64("receveur nul", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	h_eq_i64("dvm_new", dvm_new(vm, c, &a.obj), 0);
	h_eq_i64("iput-wide", dvmrt_field(vm, NULL, &a), 0);
	a.put = 0;
	a.val = 0;
	h_eq_i64("iget-wide", dvmrt_field(vm, NULL, &a), 0);
	h_eq_u64("relu", a.val, 0x1122334455667788u);
	a.is_static = 1;
	h_eq_i64("statique sur instance", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	a.is_static = 0;
	h_eq_i64("chaine", dvm_string_utf8_new(vm, "x", &a.obj), 0);
	h_eq_i64("receveur d'un autre genre", dvmrt_field(vm, NULL, &a),
		DVM_THROWN);
	a.obj = 999999u;
	h_eq_i64("receveur hostile", dvmrt_field(vm, NULL, &a), DVM_THROWN);
	d08_close(vm, file);
}

int	main(void)
{
	h_begin("d08/acces");
	h_run("champs_statiques_entiers_et_longs",
		champs_statiques_entiers_et_longs);
	h_run("champs_statiques_chaines", champs_statiques_chaines);
	h_run("champs_d_instance", champs_d_instance);
	return (h_end());
}
