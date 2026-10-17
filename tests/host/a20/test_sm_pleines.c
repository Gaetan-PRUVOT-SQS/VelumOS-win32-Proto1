#include <stdint.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "velum/libk.h"
#include "a20_test.h"
#include "startmenu.h"

static void	pleines_pose(t_smprogs *p)
{
	memset(p, 0, sizeof(*p));
	strlcpy(p->list[0].name, "Un", SM_PROG_NAME_MAX);
	strlcpy(p->list[0].path, "/system/bin/un", SM_PROG_PATH_MAX);
	strlcpy(p->list[1].name, "Deux", SM_PROG_NAME_MAX);
	strlcpy(p->list[1].path, "/system/bin/deux", SM_PROG_PATH_MAX);
	strlcpy(p->list[2].name, "Trois", SM_PROG_NAME_MAX);
	strlcpy(p->list[2].path, "/system/bin/trois", SM_PROG_PATH_MAX);
	p->count = SM_PROGS_MAX;
	h_eq_i64("trois programmes poses", sm_set_programs(p), 0);
}

static void	pleines_parcours_clavier(void)
{
	static const int32_t	order[] = {0, 1, 2, 3, 4, 5, 0};
	t_smprogs				p;
	t_startmenu				m;
	size_t					i;

	pleines_pose(&p);
	sm_init(&m);
	sm_open(&m);
	h_eq_u64("quatre tuiles", sm_tiles(), LUNA_LAUNCHER_TILES);
	i = 0;
	while (i < sizeof(order) / sizeof(order[0]))
	{
		h_eq_i64("bas: entree suivante", sm_move(&m, 1), order[i]);
		i++;
	}
	h_eq_i64("haut: reboucle", sm_move(&m, -1), SM_IDX_SHUTDOWN);
	h_eq_i64("haut: Fermer la session", sm_move(&m, -1), SM_IDX_LOGOFF);
	h_eq_i64("haut: derniere tuile", sm_move(&m, -1), 3);
}

static void	pleines_activation_et_survol(void)
{
	static const int	cmds[] = {SMC_RUN, SMC_LAUNCH_HELLO, SMC_LAUNCH_B,
		SMC_LAUNCH_C};
	t_startmenu			m;
	int32_t				i;

	sm_init(&m);
	i = 0;
	while (i < LUNA_LAUNCHER_TILES)
	{
		sm_open(&m);
		h_true(sm_visible((uint32_t)i), "tuile visible");
		h_eq_i64("survol", sm_hover(&m, i), i);
		h_eq_i64("activation", sm_activate(&m, i), cmds[i]);
		h_eq_i64("ferme apres activation", m.open, 0);
		i++;
	}
	sm_open(&m);
	h_eq_i64("survol hors table", sm_hover(&m, SM_ITEMS), SM_NO_ITEM);
	h_eq_i64("survol enorme", sm_hover(&m, INT32_MAX), SM_NO_ITEM);
	h_eq_i64("activation enorme", sm_activate(&m, INT32_MAX), SMC_NONE);
}

static void	pleines_bornes_de_la_table(void)
{
	h_true(sm_item(SM_ITEMS) == NULL, "entree hors table");
	h_true(sm_item(UINT32_MAX) == NULL, "entree enorme");
	h_true(!sm_visible(SM_ITEMS), "indice 6 invisible");
	h_true(!sm_visible(UINT32_MAX), "indice enorme invisible");
	h_eq_i64("commande inconnue", sm_prog_of(UINT32_MAX), -1);
	h_eq_i64("commande pied", sm_prog_of(SMC_SHUTDOWN), -1);
}

int	main(void)
{
	h_begin("a20/startmenu-pleines");
	h_run("lanceur: parcours clavier a quatre tuiles",
		pleines_parcours_clavier);
	h_run("lanceur: activation et survol a quatre tuiles",
		pleines_activation_et_survol);
	h_run("lanceur: bornes de la table", pleines_bornes_de_la_table);
	return (h_end());
}
