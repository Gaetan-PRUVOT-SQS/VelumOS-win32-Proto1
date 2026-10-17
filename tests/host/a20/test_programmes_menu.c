#include <stdint.h>
#include "harness.h"
#include "velum/err.h"
#include "velum/libk.h"
#include "startmenu.h"

static void	programmes_tuiles(void)
{
	t_smprogs	p;

	h_eq_i64("utf-8", sm_progs_parse("\xc3\x89" "dit;/system/bin/e\nB;/system/b"
			"\nC;/system/c\n", 44, &p), 3);
	h_eq_i64("pose", sm_set_programs(&p), 0);
	h_eq_u64("quatre tuiles", sm_tiles(), 4);
	h_eq_str("premier", sm_item(1)->label, "\xc3\x89" "dit");
	h_eq_str("troisieme", sm_item(3)->label, "C");
	h_true(sm_visible(3), "troisieme visible");
	h_eq_i64("commande C", sm_prog_of(sm_item(3)->cmd), 2);
	h_eq_i64("commande autre", sm_prog_of(SMC_RUN), -1);
}

static void	programmes_refus_et_repli(void)
{
	t_smprogs	p;

	sm_progs_default(&p);
	h_eq_i64("repli", sm_set_programs(&p), 0);
	p.count = SM_PROGS_MAX + 1;
	h_eq_i64("trop d'entrees", sm_set_programs(&p), E_INVAL);
	p.count = 0;
	h_eq_i64("aucune entree", sm_set_programs(&p), E_INVAL);
	h_eq_u64("liste inchangee apres refus", sm_tiles(), 2);
	h_eq_str("repli nom", sm_item(1)->label, "Bonjour");
	h_eq_str("repli chemin", p.list[0].path, "/system/bin/hello");
	h_true(!sm_visible(2), "deuxieme cache");
	h_true(sm_visible(SM_IDX_LOGOFF) && sm_visible(SM_IDX_SHUTDOWN),
		"pied toujours visible");
}

int	main(void)
{
	h_begin("a20/programmes-menu");
	h_run("programmes: tuiles du lanceur", programmes_tuiles);
	h_run("programmes: refus et repli", programmes_refus_et_repli);
	return (h_end());
}
