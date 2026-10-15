#include "harness.h"
#include "d11.h"

static void	essais_sans_bibliotheque(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "essais.dex", 0), 0);
	h_eq_i64("Arithmetique",
		d11_calcul(&t, "Lcom/velum/essais/Arithmetique;"), 1410065813);
	h_eq_i64("Boucles", d11_calcul(&t, "Lcom/velum/essais/Boucles;"), 5215);
	h_eq_i64("Tableaux", d11_calcul(&t, "Lcom/velum/essais/Tableaux;"), 1153);
	h_eq_i64("Aiguillages",
		d11_calcul(&t, "Lcom/velum/essais/Aiguillages;"), 306);
	d11_close(&t);
}

static void	essais_avec_bibliotheque(void)
{
	t_d11	t;

	h_eq_i64("ouverture", d11_open(&t, "essais.dex", 0), 0);
	h_eq_i64("Exceptions",
		d11_calcul(&t, "Lcom/velum/essais/Exceptions;"), 36);
	h_eq_str("Exceptions sans exception", t.exc, "");
	h_eq_i64("Appels", d11_calcul(&t, "Lcom/velum/essais/Appels;"), 59);
	h_eq_str("Appels sans exception", t.exc, "");
	h_eq_i64("Chaines", d11_calcul(&t, "Lcom/velum/essais/Chaines;"), 103);
	h_eq_str("Chaines sans exception", t.exc, "");
	h_eq_u64("pile rendue", t.vm->sp, 0);
	h_eq_u64("profondeur rendue", t.vm->depth, 0);
	d11_close(&t);
}

int	main(void)
{
	h_begin("d11/essais");
	h_run("essais_sans_bibliotheque", essais_sans_bibliotheque);
	h_run("essais_avec_bibliotheque", essais_avec_bibliotheque);
	return (h_end());
}
