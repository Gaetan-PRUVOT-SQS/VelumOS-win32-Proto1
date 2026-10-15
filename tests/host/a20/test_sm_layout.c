#include <stdint.h>
#include "velum/libk.h"
#include "harness.h"
#include "a20_test.h"
#include "layout.h"
#include "startmenu.h"

static void	layout_correspondance_des_indices(void)
{
	h_eq_i64("Fermer la session", SM_IDX_LOGOFF, LUNA_LAUNCHER_LOGOFF);
	h_eq_i64("Eteindre", SM_IDX_SHUTDOWN, LUNA_LAUNCHER_POWER);
	h_eq_i64("aucune entree", SM_NO_ITEM, LUNA_LAUNCHER_NONE);
	h_eq_i64("indices de la table", SM_ITEMS, LUNA_LAUNCHER_POWER + 1);
	h_eq_i64("une seule rangee", SM_PROGS_MAX + 1, LUNA_LAUNCHER_TILES);
}

static void	layout_valeurs_hors_table(void)
{
	t_smlayout	l;

	memset(&l, 0, sizeof(l));
	l.ntiles = 2;
	l.logoff = lay_rect(380, 240, 40, 32);
	l.poweroff = lay_rect(430, 240, 32, 32);
	h_eq_i64("indice negatif", sm_item_rect(&l, -1).w, 0);
	h_eq_i64("indice trop grand", sm_item_rect(&l, SM_ITEMS).w, 0);
	h_eq_i64("indice enorme", sm_item_rect(&l, INT32_MAX).w, 0);
	h_eq_i64("pas de geometrie", sm_item_rect(NULL, 0).w, 0);
	h_true(!lay_overlap(sm_item_rect(&l, SM_IDX_LOGOFF),
			sm_item_rect(&l, SM_IDX_SHUTDOWN)), "pied disjoint");
}

int	main(void)
{
	h_begin("a20/startmenu-layout");
	h_run("lanceur: indices du modele et du dessin",
		layout_correspondance_des_indices);
	h_run("lanceur: valeurs hors table", layout_valeurs_hors_table);
	return (h_end());
}
