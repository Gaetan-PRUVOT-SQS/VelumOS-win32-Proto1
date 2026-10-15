#include "harness.h"
#include "d08.h"

static void	creation_classes_du_noyau(void)
{
	t_dvm		*vm;
	t_dclass	*c;
	t_dref		msg;
	char		buf[96];
	t_text		out;

	out.p = buf;
	out.cap = sizeof(buf);
	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	h_eq_i64("Object", dvm_class(vm, "Ljava/lang/Object;", &c), 0);
	h_eq_i64("Class", dvm_class(vm, "Ljava/lang/Class;", &c), 0);
	h_eq_i64("CharSequence", dvm_class(vm, "Ljava/lang/CharSequence;", &c), 0);
	h_eq_i64("Throwable", dvm_class(vm, "Ljava/lang/Throwable;", &c), 0);
	h_eq_i64("String", dvm_class(vm, "Ljava/lang/String;", &c), 0);
	h_eq_str("nom", dvm_class_name(c), "Ljava/lang/String;");
	h_eq_i64("String non instanciable", dvm_new(vm, c, &msg), E_INVAL);
	h_eq_i64("inconnue", dvm_class(vm, "Lx/Absente;", &c), DVM_THROWN);
	h_eq_i64("message", dvm_throwable_message(vm, vm->pending, &msg), 0);
	h_true(dvm_string_utf8(vm, msg, out) > 0, "message lisible");
	h_eq_str("nomme", buf, "Ljava/lang/NoClassDefFoundError;: Lx/Absente;");
	dvm_destroy(vm);
	dvm_destroy(NULL);
}

static void	references_hostiles_refusees(void)
{
	t_dvm		*vm;
	t_dref		s;
	t_dstr16	v;
	t_darrview	a;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	h_true(dvm_class_of(vm, DVM_NULL) == NULL, "nul");
	h_true(dvm_class_of(vm, 1000000u) == NULL, "indice hors table");
	h_true(dvm_class_of(vm, 0xffffffffu) == NULL, "indice maximal");
	h_eq_i64("chaine", dvm_string_utf8_new(vm, "abc", &s), 0);
	h_eq_i64("chaine vue comme tableau", dvm_array_view(vm, s, &a), E_INVAL);
	h_true(dvm_payload(vm, s, dvm_class_of(vm, s)) == NULL, "charge utile");
	h_eq_i64("epingle hors table", dvm_pin(vm, 77777u), E_INVAL);
	h_eq_i64("local hors table", dvm_local(vm, 77777u), E_INVAL);
	dvm_gc(vm);
	h_true(dvm_class_of(vm, s) == NULL, "case liberee");
	h_eq_i64("case liberee lue", dvm_string_get(vm, s, &v), E_INVAL);
	h_eq_i64("instance d'une case liberee", dvm_is_instance(vm, s, NULL), 0);
	dvm_unpin(vm, s);
	dvm_destroy(vm);
}

static void	chaines_utf8_et_utf16(void)
{
	t_dvm		*vm;
	t_dref		s;
	t_dstr16	v;
	char		buf[32];
	t_text		out;

	out.p = buf;
	out.cap = sizeof(buf);
	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	h_eq_i64("utf8", dvm_string_utf8_new(vm, "h\xc3\xa9\xe2\x82\xac\xf0\x9f"
			"\x98\x80", &s), 0);
	h_eq_i64("get", dvm_string_get(vm, s, &v), 0);
	h_eq_u64("cinq unites", v.n, 5);
	h_eq_u64("paire haute", v.p[3], 0xd83d);
	h_eq_i64("retour utf8", dvm_string_utf8(vm, s, out), 10);
	h_eq_str("aller-retour", buf, "h\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80");
	out.cap = 10;
	h_eq_i64("tampon trop petit", dvm_string_utf8(vm, s, out), E_OVERFLOW);
	h_eq_i64("octet invalide", dvm_string_utf8_new(vm, "a\xff", &s), E_INVAL);
	h_eq_i64("forme longue", dvm_string_utf8_new(vm, "\xc0\x80", &s), E_INVAL);
	h_eq_i64("tronquee", dvm_string_utf8_new(vm, "\xe2\x82", &s), E_INVAL);
	h_eq_i64("vide", dvm_string_utf8_new(vm, "", &s), 0);
	h_eq_i64("vide en utf8", dvm_string_utf8(vm, s, out), 0);
	dvm_destroy(vm);
}

static void	tableaux_par_descripteur(void)
{
	t_dvm		*vm;
	t_dref		a;
	t_dref		b;
	t_darrview	v;
	t_dclass	*c;

	h_eq_i64("dvm_create", dvm_create(&vm, NULL), 0);
	h_eq_i64("[J", dvm_array_new(vm, "[J", 3, &a), 0);
	h_eq_i64("vue", dvm_array_view(vm, a, &v), 0);
	h_eq_u64("largeur longue", v.width, 8);
	h_eq_i64("[Z", dvm_array_new(vm, "[Z", 0, &a), 0);
	h_eq_i64("[[I", dvm_array_new(vm, "[[I", 2, &b), 0);
	h_eq_i64("[String", dvm_array_new(vm, "[Ljava/lang/String;", 2, &a), 0);
	h_eq_i64("classe", dvm_class(vm, "[Ljava/lang/Object;", &c), 0);
	h_eq_i64("covariance", dvm_is_instance(vm, a, c), 1);
	h_eq_i64("tableau de tableaux", dvm_is_instance(vm, b, c), 1);
	h_eq_i64("[I n'est pas [J", dvm_is_instance(vm, b, dvm_class_of(vm, a)), 0);
	h_eq_i64("negatif", dvm_array_new(vm, "[I", -1, &a), DVM_THROWN);
	h_eq_i64("genre inconnu", dvm_array_new(vm, "[Q", 1, &a), E_INVAL);
	h_eq_i64("primitif suivi", dvm_array_new(vm, "[II", 1, &a), E_INVAL);
	h_eq_i64("element absent", dvm_array_new(vm, "[Lx/Y;", 1, &a), DVM_THROWN);
	h_eq_i64("pas un tableau", dvm_array_new(vm, "I", 1, &a), E_INVAL);
	dvm_destroy(vm);
}

int	main(void)
{
	h_begin("d08/objets");
	h_run("creation_classes_du_noyau", creation_classes_du_noyau);
	h_run("references_hostiles_refusees", references_hostiles_refusees);
	h_run("chaines_utf8_et_utf16", chaines_utf8_et_utf16);
	h_run("tableaux_par_descripteur", tableaux_par_descripteur);
	return (h_end());
}
