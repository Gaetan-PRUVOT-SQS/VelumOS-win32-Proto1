#include "harness.h"
#include "fichiers_layout.h"

static void	fenetre_dans_l_ecran_1024_768(void)
{
	h_true(FF_X >= 0 && FF_X + FF_W <= 1024, "largeur dans 1024 px");
	h_true(FF_Y >= 0 && FF_Y + FF_H_PX <= 768 - 30, "hauteur au-dessus de la barre");
}

static void	contenu_dans_le_client(void)
{
	int	client_h;
	int	rows_bottom;

	client_h = FF_H_PX - VT_CAPTION_H;
	rows_bottom = FF_ACTIONS_Y + FF_ACTION_ROW + FF_ACTION_H;
	h_true(rows_bottom <= client_h, "boutons sous le bas du client");
	h_true(FF_LIST_Y + FF_LIST_H <= FF_NAME_Y, "liste au-dessus du champ nom");
	h_true(FF_NAME_Y + FF_NAME_H <= FF_STATUS_Y, "champ nom au-dessus du statut");
	h_true(FF_STATUS_Y + FF_LINE_H <= FF_ACTIONS_Y, "statut au-dessus des boutons");
	h_true(FF_MAIN_X + 2 * FF_ACTION_STEP + FF_ACTION_W <= FF_W,
		"3 colonnes de boutons dans la largeur");
	h_true(FF_MAIN_X + FF_MAIN_W <= FF_W, "zone principale dans la largeur");
	h_true(FF_SIDE_X + FF_SIDE_W <= FF_MAIN_X, "barre latérale avant la zone principale");
	h_true(FF_SIDE_FIRST_Y + 2 * FF_SIDE_STEP + FF_SIDE_BTN_H <= client_h,
		"favoris dans le client");
}

int	main(void)
{
	h_begin("e11/disposition");
	h_run("disposition: écran 1024x768 (QEMU)", fenetre_dans_l_ecran_1024_768);
	h_run("disposition: contenu dans le client", contenu_dans_le_client);
	return (h_end());
}
