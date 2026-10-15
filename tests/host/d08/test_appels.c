#include "harness.h"
#include "d08.h"

static void	resolution_virtuelle_et_directe(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dclass	*c;
	t_dinvoke	inv;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("Fille", dvm_class(vm, "Ld08/Fille;", &c), 0);
	inv = (t_dinvoke){0, DVM_NULL, NULL, DIK_VIRTUAL};
	inv.method_idx = (uint32_t)d08_method_idx(vm, "Ld08/Base;", "valeur");
	h_eq_i64("this nul", dvmrt_resolve(vm, NULL, &inv), DVM_THROWN);
	h_eq_i64("dvm_new", dvm_new(vm, c, &inv.self), 0);
	h_eq_i64("virtuel", dvmrt_resolve(vm, NULL, &inv), 0);
	h_true(inv.target && inv.target->cls == c, "methode de la fille");
	inv.kind = DIK_DIRECT;
	h_eq_i64("direct", dvmrt_resolve(vm, NULL, &inv), 0);
	h_true(inv.target && inv.target->cls != c, "methode de la mere");
	inv.kind = DIK_SUPER;
	h_eq_i64("super sans appelant", dvmrt_resolve(vm, NULL, &inv), 0);
	inv.self = 123456u;
	h_eq_i64("this hostile", dvmrt_resolve(vm, NULL, &inv), DVM_THROWN);
	d08_close(vm, file);
}

static void	resolution_statique_contre_instance(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dclass	*c;
	t_dinvoke	inv;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("Fille", dvm_class(vm, "Ld08/Fille;", &c), 0);
	inv = (t_dinvoke){0, DVM_NULL, NULL, DIK_STATIC};
	inv.method_idx = (uint32_t)d08_method_idx(vm, "Ld08/Base;", "somme");
	h_eq_i64("statique", dvmrt_resolve(vm, NULL, &inv), 0);
	h_true(inv.target && inv.target->ins == 3, "cible statique");
	h_eq_i64("dvm_new", dvm_new(vm, c, &inv.self), 0);
	inv.kind = DIK_VIRTUAL;
	h_eq_i64("instance sur statique", dvmrt_resolve(vm, NULL, &inv),
		DVM_THROWN);
	inv.kind = DIK_STATIC;
	inv.method_idx = (uint32_t)d08_method_idx(vm, "Ld08/Base;", "valeur");
	h_eq_i64("statique sur instance", dvmrt_resolve(vm, NULL, &inv),
		DVM_THROWN);
	inv.kind = 9;
	h_eq_i64("genre inconnu", dvmrt_resolve(vm, NULL, &inv), E_INVAL);
	inv.kind = DIK_STATIC;
	inv.method_idx = 0x00ffffffu;
	h_eq_i64("indice hors table", dvmrt_resolve(vm, NULL, &inv), E_INVAL);
	d08_close(vm, file);
}

static void	tableaux_d_octets(void)
{
	t_dvm	*vm;
	t_span	file;
	t_daacc	a;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	a = (t_daacc){DVM_NULL, 1, 0xfffffff0u, DAK_BYTE, 1};
	h_eq_i64("tableau nul", dvmrt_array(vm, &a), DVM_THROWN);
	h_eq_i64("[B", dvm_array_new(vm, "[B", 2, &a.arr), 0);
	h_eq_i64("aput-byte", dvmrt_array(vm, &a), 0);
	a.put = 0;
	h_eq_i64("aget-byte", dvmrt_array(vm, &a), 0);
	h_eq_u64("signe etendu", a.val, 0xfffffff0u);
	a.index = 2;
	h_eq_i64("borne haute", dvmrt_array(vm, &a), DVM_THROWN);
	a.index = -1;
	h_eq_i64("indice negatif", dvmrt_array(vm, &a), DVM_THROWN);
	a.index = 0;
	a.kind = DAK_INT;
	h_eq_i64("genre faux", dvmrt_array(vm, &a), DVM_THROWN);
	d08_close(vm, file);
}

static void	tableaux_d_objets(void)
{
	t_dvm		*vm;
	t_span		file;
	t_daacc		a;
	t_dref		s;
	uint32_t	len;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("chaine", dvm_string_utf8_new(vm, "x", &s), 0);
	h_eq_i64("dvm_local", dvm_local(vm, s), 0);
	a = (t_daacc){DVM_NULL, 0, s, DAK_OBJECT, 1};
	h_eq_i64("[[I", dvm_array_new(vm, "[[I", 1, &a.arr), 0);
	h_eq_i64("ArrayStoreException", dvmrt_array(vm, &a), DVM_THROWN);
	h_eq_i64("longueur", dvmrt_array_length(vm, a.arr, &len), 0);
	h_eq_u64("un element", len, 1);
	h_eq_i64("[String", dvm_array_new(vm, "[Ljava/lang/String;", 1, &a.arr),
		0);
	h_eq_i64("aput-object", dvmrt_array(vm, &a), 0);
	a.val = 555555u;
	h_eq_i64("reference hostile", dvmrt_array(vm, &a), DVM_THROWN);
	h_eq_i64("longueur d'un nul", dvmrt_array_length(vm, DVM_NULL, &len),
		DVM_THROWN);
	h_eq_i64("longueur d'une chaine", dvmrt_array_length(vm, s, &len),
		DVM_THROWN);
	d08_close(vm, file);
}

int	main(void)
{
	h_begin("d08/appels");
	h_run("resolution_virtuelle_et_directe", resolution_virtuelle_et_directe);
	h_run("resolution_statique_contre_instance",
		resolution_statique_contre_instance);
	h_run("tableaux_d_octets", tableaux_d_octets);
	h_run("tableaux_d_objets", tableaux_d_objets);
	return (h_end());
}
