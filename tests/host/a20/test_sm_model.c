#include <stdint.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "a20_test.h"
#include "startmenu.h"

static void	model_ferme_et_ouverture(void)
{
	t_startmenu	m;

	sm_init(&m);
	h_eq_i64("ferme au depart", m.open, 0);
	h_eq_i64("touche ignoree si ferme", sm_key(&m, VK_DOWN), SMC_NONE);
	h_eq_i64("survol ignore si ferme", sm_hover(&m, 1), SM_NO_ITEM);
	h_eq_i64("activation ignoree si ferme", sm_activate(&m, 1), SMC_NONE);
	h_eq_i64("deplacement ignore si ferme", sm_move(&m, 1), SM_NO_ITEM);
	sm_open(&m);
	h_eq_i64("ouvert", m.open, 1);
	h_eq_i64("rien de survole", m.hot, SM_NO_ITEM);
	h_eq_i64("aucun focus clavier", m.focus, SM_NO_ITEM);
}

static void	model_fleches_et_bouclage(void)
{
	t_startmenu	m;

	sm_init(&m);
	sm_open(&m);
	h_eq_i64("bas: Executer", sm_move(&m, 1), SM_IDX_RUN);
	h_eq_i64("bas: Bonjour", sm_move(&m, 1), 1);
	h_eq_i64("bas: saute les tuiles absentes", sm_move(&m, 1), SM_IDX_LOGOFF);
	h_eq_i64("bas: Eteindre", sm_move(&m, 1), SM_IDX_SHUTDOWN);
	h_eq_i64("bas: reboucle", sm_move(&m, 1), SM_IDX_RUN);
	h_eq_i64("haut: reboucle", sm_move(&m, -1), SM_IDX_SHUTDOWN);
	h_eq_i64("haut: Fermer la session", sm_move(&m, -1), SM_IDX_LOGOFF);
	h_eq_i64("haut: saute les tuiles absentes", sm_move(&m, -1), 1);
	sm_key(&m, VK_HOME);
	h_eq_i64("Debut", m.focus, SM_IDX_RUN);
	sm_key(&m, VK_END);
	h_eq_i64("Fin", m.focus, SM_IDX_SHUTDOWN);
	sm_close(&m);
	sm_open(&m);
	h_eq_i64("haut depuis rien: derniere entree", sm_move(&m, -1),
		SM_IDX_SHUTDOWN);
}

static void	model_touches_equivalentes(void)
{
	t_startmenu	m;

	sm_init(&m);
	sm_open(&m);
	sm_key(&m, VK_RIGHT);
	h_eq_i64("Droite avance", m.focus, SM_IDX_RUN);
	sm_key(&m, VK_TAB);
	h_eq_i64("Tab avance", m.focus, 1);
	sm_key(&m, VK_LEFT);
	h_eq_i64("Gauche recule", m.focus, SM_IDX_RUN);
	sm_key(&m, VK_DOWN);
	sm_key(&m, VK_UP);
	h_eq_i64("Haut recule", m.focus, SM_IDX_RUN);
	h_eq_i64("touche inconnue sans effet", sm_key(&m, VK_F1), SMC_NONE);
	h_eq_i64("focus inchange", m.focus, SM_IDX_RUN);
}

int	main(void)
{
	h_begin("a20/startmenu-model");
	h_run("lanceur: ferme et ouverture", model_ferme_et_ouverture);
	h_run("lanceur: fleches et bouclage", model_fleches_et_bouclage);
	h_run("lanceur: touches equivalentes", model_touches_equivalentes);
	return (h_end());
}
