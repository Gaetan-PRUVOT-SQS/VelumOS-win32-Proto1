#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "a20_wm.h"
#include "tasklist.h"

static void	tasklist_filtrage_par_style_et_etat(void)
{
	static const uint32_t	hidden[] = {WS_TOOLWINDOW, WS_DESKTOP, WS_APPBAR,
		WS_POPUP, WS_NOACTIVATE, WS_DEFAULT | WS_TOOLWINDOW};
	t_tasklist				l;
	t_wminfo				w;
	uint32_t				i;

	tl_init(&l);
	i = 0;
	while (i < sizeof(hidden) / sizeof(hidden[0]))
	{
		mk_info(&w, 10 + i, hidden[i], WSTATE_NORMAL);
		h_eq_i64("style masque", tl_apply(&l, WMS_WIN_ADD, &w), 0);
		i++;
	}
	h_eq_u64("rien dans la liste", l.count, 0);
	mk_info(&w, 5, WS_DEFAULT, WSTATE_NORMAL);
	h_eq_i64("fenetre normale", tl_apply(&l, WMS_WIN_ADD, &w), 1);
	mk_info(&w, 6, WS_CAPTION, WSTATE_MIN);
	h_eq_i64("reduite: visible", tl_apply(&l, WMS_WIN_ADD, &w), 1);
	mk_info(&w, 7, WS_DEFAULT, WSTATE_HIDDEN);
	h_eq_i64("cachee: masquee", tl_apply(&l, WMS_WIN_ADD, &w), 0);
	h_eq_u64("deux fenetres", l.count, 2);
}

static void	tasklist_ajout_maj_suppression(void)
{
	t_tasklist	l;
	t_wminfo	w;

	tl_init(&l);
	mk_info(&w, 1, WS_DEFAULT, WSTATE_NORMAL);
	tl_apply(&l, WMS_WIN_ADD, &w);
	w.h.window = 2;
	tl_apply(&l, WMS_WIN_ADD, &w);
	w.h.window = 3;
	tl_apply(&l, WMS_WIN_ADD, &w);
	h_eq_i64("ordre d'arrivee", l.list[2].id, 3);
	w.h.window = 2;
	h_eq_i64("suppression du milieu", tl_apply(&l, WMS_WIN_DEL, &w), 1);
	h_eq_u64("deux restent", l.count, 2);
	h_eq_i64("ordre conserve", l.list[1].id, 3);
	h_eq_i64("deja supprimee", tl_apply(&l, WMS_WIN_DEL, &w), 0);
	h_eq_i64("recherche apres suppression", tl_find(&l, 3), 1);
	h_eq_i64("recherche d'une absente", tl_find(&l, 2), -1);
	w.h.window = 0;
	h_eq_i64("identifiant zero", tl_apply(&l, WMS_WIN_ADD, &w), 0);
	w.h.window = 1;
	h_eq_i64("type inconnu", tl_apply(&l, WMS_KEY, &w), 0);
}

int	main(void)
{
	h_begin("a20/tasklist");
	h_run("taches: filtrage par style et etat",
		tasklist_filtrage_par_style_et_etat);
	h_run("taches: ajout, mise a jour, suppression",
		tasklist_ajout_maj_suppression);
	return (h_end());
}
