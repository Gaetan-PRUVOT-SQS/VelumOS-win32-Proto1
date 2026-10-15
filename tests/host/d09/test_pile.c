#include "harness.h"
#include "d09.h"

static void	recursion_au_plafond_puis_reprise(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 16, 0);
	ret = 0;
	g_fk.args[0] = 15;
	h_eq_i64("15 appels", fk_calc("Ld09/Rec;", "descend", &ret), 0);
	h_eq_u64("profondeur 16 atteinte", ret, 15);
	g_fk.args[0] = 16;
	h_eq_i64("17 cadres", fk_calc("Ld09/Rec;", "descend", &ret), DVM_THROWN);
	h_eq_str("erreur", fk_pending(), "Ljava/lang/StackOverflowError;");
	fk_check("pile apres debordement");
	g_fk.vm.pending = DVM_NULL;
	h_eq_i64("reprise", fk_calc("Ld09/Rec;", "reprise", &ret), 0);
	h_eq_u64("reprise apres StackOverflowError", ret, 5);
	fk_check("pile apres reprise");
	fk_end();
}

static void	pile_de_registres_pleine(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 1000, 0);
	g_fk.vm.stack_words = 40;
	g_fk.args[0] = 5;
	ret = 0;
	h_eq_i64("six cadres", fk_calc("Ld09/Rec;", "descend", &ret), 0);
	h_eq_u64("six cadres", ret, 5);
	g_fk.args[0] = 6;
	h_eq_i64("sept cadres", fk_calc("Ld09/Rec;", "descend", &ret),
		DVM_THROWN);
	h_eq_str("erreur", fk_pending(), "Ljava/lang/StackOverflowError;");
	fk_check("pile");
	fk_end();
}

static void	budget_epuise(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 1000);
	ret = 0;
	h_eq_i64("boucle", fk_calc("Ld09/Rec;", "boucle", &ret), E_TIMEOUT);
	h_eq_u64("instructions comptees", g_fk.vm.spent, 1000);
	fk_check("pile");
	fk_end();
	fk_init("d09.dex", 8, 0);
	g_fk.args[0] = 3;
	h_eq_i64("sans budget", fk_calc("Ld09/Rec;", "descend", &ret), 0);
	h_eq_u64("compteur", g_fk.vm.spent, 3 * 6 + 3);
	fk_end();
}

int	main(void)
{
	h_begin("d09/pile");
	h_run("recursion_au_plafond_puis_reprise",
		recursion_au_plafond_puis_reprise);
	h_run("pile_de_registres_pleine", pile_de_registres_pleine);
	h_run("budget_epuise", budget_epuise);
	return (h_end());
}
