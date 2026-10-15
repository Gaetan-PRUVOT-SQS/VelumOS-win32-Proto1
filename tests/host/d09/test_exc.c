#include "harness.h"
#include "d09.h"

static void	attrapee_dans_le_cadre(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 0;
	h_eq_i64("rc", fk_calc("Ld09/Exc;", "dansCadre", &ret), 0);
	h_eq_u64("gestionnaire joue", ret, 7);
	h_eq_u64("move-exception vide pending", g_fk.vm.pending, 0);
	fk_check("pile");
	fk_end();
}

static void	attrapee_dans_l_appelant(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 0;
	h_eq_i64("rc", fk_calc("Ld09/Exc;", "dansAppelant", &ret), 0);
	h_eq_u64("gestionnaire de l'appelant", ret, 42);
	h_eq_u64("pending vide", g_fk.vm.pending, 0);
	fk_check("pile");
	fk_end();
}

static void	non_attrapee(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 77;
	h_eq_i64("rc", fk_calc("Ld09/Exc;", "nonAttrapee", &ret), DVM_THROWN);
	h_eq_str("throw d'un nul", fk_pending(), FK_NPE);
	h_eq_u64("ret intact", ret, 77);
	fk_check("pile");
	fk_end();
}

static void	tout_attraper(void)
{
	uint64_t	ret;

	fk_init("d09.dex", 8, 0);
	ret = 0;
	h_eq_i64("rc", fk_calc("Ld09/Exc;", "finalement", &ret), 0);
	h_eq_u64("gestionnaire tout", ret, 9);
	fk_check("pile");
	fk_end();
}

int	main(void)
{
	h_begin("d09/exc");
	h_run("attrapee_dans_le_cadre", attrapee_dans_le_cadre);
	h_run("attrapee_dans_l_appelant", attrapee_dans_l_appelant);
	h_run("non_attrapee", non_attrapee);
	h_run("tout_attraper", tout_attraper);
	return (h_end());
}
