#include "harness.h"
#include "d11.h"

static void	exception_dans_oncreate(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("droid_start leve",
		droid_start(&t.d, "Lcom/velum/d11/Casse;"), DVM_THROWN);
	h_eq_str("phrase classe : message", t.d.error,
		"java.lang.IllegalStateException : panne");
	h_eq_u64("exception retiree", t.vm->pending, DVM_NULL);
	h_eq_u64("pile rendue", t.vm->sp, 0);
	h_eq_i64("machine reutilisable", d11_i2(&t, M_ABS, (uint32_t)-5, 0), 5);
	h_eq_i64("arret apres echec", droid_stop(&t.d), 0);
	h_eq_i64("activite absente", droid_start(&t.d, "Lx/Absente;"),
		DVM_THROWN);
	h_true(d11_has(t.d.error, "Absente"), "la classe absente est nommee");
	h_eq_u64("exception retiree bis", t.vm->pending, DVM_NULL);
	d11_close(&t);
}

static void	exception_dans_onclick(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("droid_start", droid_start(&t.d, "Lcom/velum/d11/Clic;"), 0);
	h_eq_str("titre", t.d.title, "Titre");
	h_eq_u64("racine", t.d.root, 1);
	h_eq_i64("clic leve", droid_click(&t.d, 1), DVM_THROWN);
	h_true(d11_has(t.d.error, "java.lang.ArithmeticException"),
		"d->error nomme ArithmeticException");
	h_eq_u64("exception retiree", t.vm->pending, DVM_NULL);
	dvm_gc(t.vm);
	h_eq_i64("second clic leve encore", droid_click(&t.d, 1), DVM_THROWN);
	h_eq_u64("pas fini avant onDestroy", t.d.finished, 0);
	h_eq_i64("droid_stop", droid_stop(&t.d), 0);
	h_eq_u64("onDestroy a appele finish", t.d.finished, 1);
	d11_close(&t);
}

static void	plafonds_vues_et_enfants(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("65 vues", droid_start(&t.d, "Lcom/velum/d11/Foule;"),
		DVM_THROWN);
	h_eq_str("OutOfMemoryError", t.d.error,
		"java.lang.OutOfMemoryError : trop de vues");
	h_eq_u64("64 vues gardees", t.d.nviews, DROID_VIEWS_MAX);
	h_eq_i64("arret", droid_stop(&t.d), 0);
	h_eq_i64("17 enfants", droid_start(&t.d, "Lcom/velum/d11/Famille;"),
		DVM_THROWN);
	h_eq_str("IllegalStateException", t.d.error,
		"java.lang.IllegalStateException : trop d'enfants");
	h_eq_u64("16 enfants gardes", t.d.views[0].nkids, DROID_KIDS_MAX);
	h_eq_u64("18 vues", t.d.nviews, 18);
	d11_close(&t);
}

static void	hors_sous_ensemble(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "d11.dex", 0), 0);
	h_eq_i64("classe absente",
		droid_start(&t.d, "Lcom/velum/d11/ManqueClasse;"), DVM_THROWN);
	h_true(d11_has(t.d.error, "java.lang.NoClassDefFoundError"),
		"NoClassDefFoundError");
	h_true(d11_has(t.d.error, "Landroid/widget/Toast;"), "Toast nomme");
	h_eq_i64("arret", droid_stop(&t.d), 0);
	h_eq_i64("methode absente",
		droid_start(&t.d, "Lcom/velum/d11/ManqueMethode;"), DVM_THROWN);
	h_true(d11_has(t.d.error, "java.lang.NoSuchMethodError"),
		"NoSuchMethodError");
	h_true(d11_has(t.d.error, "setTextColor(I)V"), "setTextColor nomme");
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/fautes");
	h_run("exception_dans_oncreate", exception_dans_oncreate);
	h_run("exception_dans_onclick", exception_dans_onclick);
	h_run("plafonds_vues_et_enfants", plafonds_vues_et_enfants);
	h_run("hors_sous_ensemble", hors_sous_ensemble);
	return (h_end());
}
