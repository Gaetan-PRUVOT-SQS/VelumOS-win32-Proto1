#include "harness.h"
#include "d09.h"

static void	essai(const char *cls, uint64_t want)
{
	uint64_t	ret;

	fk_init("essais.dex", 32, 0);
	ret = 0;
	h_eq_i64(cls, fk_calc(cls, "calcul", &ret), 0);
	h_eq_u64(cls, ret, want);
	fk_check(cls);
	fk_end();
}

static void	essais_de_resultats_toml(void)
{
	essai("Lcom/velum/essais/Arithmetique;", 1410065813);
	essai("Lcom/velum/essais/Boucles;", 5215);
	essai("Lcom/velum/essais/Tableaux;", 1153);
	essai("Lcom/velum/essais/Aiguillages;", 306);
}

int	main(void)
{
	h_begin("d09/essais");
	h_run("essais_de_resultats_toml", essais_de_resultats_toml);
	return (h_end());
}
