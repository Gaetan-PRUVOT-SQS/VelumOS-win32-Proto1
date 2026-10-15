#include "harness.h"
#include "d08.h"

static void	liaison_et_clinit_une_fois(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dclass	*c[3];
	t_dref		o;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("second DEX", dvm_load_dex(vm, file), E_NOTSUP);
	h_eq_i64("Fille", dvm_class(vm, "Ld08/Fille;", &c[0]), 0);
	h_eq_u64("clinit de Base appele", g_d08call.calls, 1);
	h_eq_str("methode appelee", g_d08call.last->name, "<clinit>");
	h_eq_i64("Base", dvm_class(vm, "Ld08/Base;", &c[1]), 0);
	h_eq_i64("Marque", dvm_class(vm, "Ld08/Marque;", &c[2]), 0);
	h_eq_u64("clinit une seule fois", g_d08call.calls, 1);
	h_eq_i64("dvm_new", dvm_new(vm, c[0], &o), 0);
	h_eq_i64("instance de la mere", dvm_is_instance(vm, o, c[1]), 1);
	h_eq_i64("instance de l'interface", dvm_is_instance(vm, o, c[2]), 1);
	h_eq_i64("interface non instanciable", dvm_new(vm, c[2], &o), E_INVAL);
	h_eq_i64("absente", dvm_class(vm, "Ld08/Absente;", &c[2]), DVM_THROWN);
	d08_close(vm, file);
}

static void	methodes_par_nom_et_signature(void)
{
	t_dvm			*vm;
	t_span			file;
	t_dclass		*c;
	const t_dmethod	*m;
	t_dname			nm;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	h_eq_i64("Fille", dvm_class(vm, "Ld08/Fille;", &c), 0);
	nm.name = "valeur";
	nm.sig = "()I";
	h_eq_i64("dvm_method", dvm_method(vm, c, &nm, &m), 0);
	h_true(m->cls == c && m->registers == 2 && m->ins == 1, "redefinie");
	h_eq_i64("debut d'instruction", dvm_method_start(m, 0), 1);
	h_eq_i64("pc hors du code", dvm_method_start(m, 4096), 0);
	nm.name = "somme";
	nm.sig = "(IJ)I";
	h_eq_i64("heritee", dvm_method(vm, c, &nm, &m), 0);
	h_true(m->cls != c && m->ins == 3, "trois mots d'arguments");
	h_eq_str("signature reconstruite", m->sig, "(IJ)I");
	nm.sig = "(I)I";
	h_eq_i64("signature absente", dvm_method(vm, c, &nm, &m), E_NOENT);
	d08_close(vm, file);
}

static void	code_invalide_refuse(void)
{
	t_dvm		*vm;
	t_span		file;
	t_dclass	*c;
	uint8_t		*p;
	uint32_t	at;

	h_eq_i64("dvm_load_dex", d08_open(&vm, &file), 0);
	p = (uint8_t *)(uintptr_t)file.p;
	h_eq_i64("Marque avant", dvm_class(vm, "Ld08/Marque;", &c), 0);
	at = 0;
	while (at + 4 < file.len && !(p[at] == 0x12 && p[at + 1] == 0x10
			&& p[at + 2] == 0x0f && p[at + 3] == 0x00))
		at++;
	h_true(at + 4 < file.len, "code de valeur() trouve");
	p[at] = 0x3e;
	h_eq_i64("classe refusee", dvm_class(vm, "Ld08/Base;", &c), DVM_THROWN);
	h_true(dvm_class_of(vm, vm->pending) != NULL, "exception en attente");
	h_eq_i64("fille refusee", dvm_class(vm, "Ld08/Fille;", &c), DVM_THROWN);
	h_eq_u64("aucun clinit joue", g_d08call.calls, 0);
	h_eq_i64("machine utilisable", dvm_array_new(vm, "[I", 4, &at), 0);
	d08_close(vm, file);
}

int	main(void)
{
	h_begin("d08/dex");
	h_run("liaison_et_clinit_une_fois", liaison_et_clinit_une_fois);
	h_run("methodes_par_nom_et_signature", methodes_par_nom_et_signature);
	h_run("code_invalide_refuse", code_invalide_refuse);
	return (h_end());
}
