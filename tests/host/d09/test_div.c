#include "harness.h"
#include "d09.h"

static void	vaut(const char *name, uint64_t want)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 0;
	h_eq_i64(name, fk_calc("Ld09/Div;", name, &ret), 0);
	h_eq_u64(name, ret, want);
	fk_check(name);
	fk_end();
}

static void	leve(const char *name, const char *desc)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 0;
	h_eq_i64(name, fk_calc("Ld09/Div;", name, &ret), DVM_THROWN);
	h_eq_str(name, fk_pending(), desc);
	fk_check(name);
	fk_end();
}

static void	champs_appels_larges_tableaux_genres(void)
{
	vaut("champs", 302);
	vaut("large", 0x10000000c);
	vaut("rempli", 8);
	vaut("genre", 1);
}

static void	levees_par_l_interpreteur(void)
{
	leve("mauvaisGenre", "Ljava/lang/ClassCastException;");
	leve("moniteurNul", FK_NPE);
}

int	main(void)
{
	h_begin("d09/div");
	h_run("champs_appels_larges_tableaux_genres",
		champs_appels_larges_tableaux_genres);
	h_run("levees_par_l_interpreteur", levees_par_l_interpreteur);
	return (h_end());
}
