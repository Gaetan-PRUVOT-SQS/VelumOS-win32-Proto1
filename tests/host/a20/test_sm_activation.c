#include <stdint.h>
#include "harness.h"
#include "velum/abi/abi_input.h"
#include "a20_test.h"
#include "startmenu.h"

static void	model_survol_et_focus(void)
{
	t_startmenu	m;

	sm_init(&m);
	sm_open(&m);
	sm_move(&m, 1);
	h_eq_i64("survol d'une tuile", sm_hover(&m, 1), 1);
	h_eq_i64("le survol efface le focus", m.focus, SM_NO_ITEM);
	sm_move(&m, 1);
	h_eq_i64("le clavier efface le survol", m.hot, SM_NO_ITEM);
	h_eq_i64("survol d'une tuile absente", sm_hover(&m, 2), SM_NO_ITEM);
	h_eq_i64("survol hors table", sm_hover(&m, SM_ITEMS), SM_NO_ITEM);
	h_eq_i64("survol negatif", sm_hover(&m, -7), SM_NO_ITEM);
}

static void	model_activation(void)
{
	t_startmenu	m;

	sm_init(&m);
	sm_open(&m);
	h_eq_i64("Entree sans focus", sm_key(&m, VK_RETURN), SMC_NONE);
	h_eq_i64("toujours ouvert", m.open, 1);
	h_eq_i64("tuile absente", sm_activate(&m, 2), SMC_NONE);
	h_eq_i64("hors table", sm_activate(&m, SM_ITEMS), SMC_NONE);
	h_eq_i64("toujours ouvert", m.open, 1);
	sm_move(&m, 1);
	h_eq_i64("Entree: Executer", sm_key(&m, VK_RETURN), SMC_RUN);
	h_eq_i64("ferme apres activation", m.open, 0);
	sm_open(&m);
	sm_move(&m, 1);
	sm_move(&m, 1);
	h_eq_i64("Espace: Bonjour", sm_key(&m, VK_SPACE), SMC_LAUNCH_HELLO);
}

static void	model_activation_du_pied(void)
{
	t_startmenu	m;

	sm_init(&m);
	sm_open(&m);
	sm_hover(&m, SM_IDX_LOGOFF);
	h_eq_i64("Entree sur le survol", sm_key(&m, VK_RETURN), SMC_LOGOFF);
	sm_open(&m);
	sm_key(&m, VK_END);
	h_eq_i64("Eteindre", sm_key(&m, VK_RETURN), SMC_SHUTDOWN);
	sm_open(&m);
	sm_key(&m, VK_ESCAPE);
	h_eq_i64("Echap ferme", m.open, 0);
	sm_open(&m);
	sm_key(&m, VK_LWIN);
	h_eq_i64("Windows ferme", m.open, 0);
}

int	main(void)
{
	h_begin("a20/startmenu-activation");
	h_run("lanceur: survol et focus", model_survol_et_focus);
	h_run("lanceur: activation des tuiles", model_activation);
	h_run("lanceur: activation du pied", model_activation_du_pied);
	return (h_end());
}
