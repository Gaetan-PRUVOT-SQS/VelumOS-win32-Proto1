#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "a20_wm.h"
#include "tasklist.h"

static void	state_activation_exclusive(void)
{
	t_tasklist	l;
	t_wminfo	w;

	tl_init(&l);
	mk_info(&w, 1, WS_DEFAULT, WSTATE_NORMAL);
	tl_apply(&l, WMS_WIN_ADD, &w);
	w.h.window = 2;
	tl_apply(&l, WMS_WIN_ADD, &w);
	h_true(tl_active(&l) == NULL, "aucune active au depart");
	w.active = 1;
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_eq_i64("la seconde est active", tl_active(&l)->id, 2);
	w.h.window = 1;
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_eq_i64("la premiere prend l'activation", tl_active(&l)->id, 1);
	h_eq_i64("la seconde ne l'est plus", l.list[1].active, 0);
	w.active = 0;
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_true(tl_active(&l) == NULL, "plus aucune active");
}

static void	state_mise_a_jour_et_masquage(void)
{
	t_tasklist	l;
	t_wminfo	w;

	tl_init(&l);
	mk_info(&w, 4, WS_DEFAULT, WSTATE_NORMAL);
	h_eq_i64("mise a jour d'une inconnue visible", tl_apply(&l, WMS_WIN_UPD,
			&w), 1);
	strlcpy(w.title, "Titre 2", sizeof(w.title));
	w.state = WSTATE_MIN;
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_eq_str("titre mis a jour", l.list[0].title, "Titre 2");
	h_eq_u64("etat mis a jour", l.list[0].state, WSTATE_MIN);
	h_eq_u64("pas de doublon", l.count, 1);
	w.state = WSTATE_HIDDEN;
	h_eq_i64("devient cachee: retiree", tl_apply(&l, WMS_WIN_UPD, &w), 1);
	h_eq_u64("liste vide", l.count, 0);
	h_eq_i64("cachee et absente", tl_apply(&l, WMS_WIN_UPD, &w), 0);
}

static void	state_titres_hostiles(void)
{
	t_tasklist	l;
	t_wminfo	w;

	tl_init(&l);
	mk_info(&w, 1, WS_DEFAULT, WSTATE_NORMAL);
	memset(w.title, 'A', sizeof(w.title));
	tl_apply(&l, WMS_WIN_ADD, &w);
	h_eq_u64("titre non termine: tronque et termine", strlen(l.list[0].title),
		TASK_TITLE_MAX - 1);
	memcpy(w.title, "a\001b\377c\303", 7);
	w.title[7] = '\0';
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_eq_str("controles et octets invalides remplaces", l.list[0].title,
		"a?b?c?");
	memset(w.title, 0, sizeof(w.title));
	tl_apply(&l, WMS_WIN_UPD, &w);
	h_eq_str("titre vide", l.list[0].title, "");
}

static void	state_plafond_de_fenetres(void)
{
	t_tasklist	l;
	t_wminfo	w;
	uint32_t	i;

	tl_init(&l);
	mk_info(&w, 1, WS_DEFAULT, WSTATE_NORMAL);
	i = 1;
	while (i <= TASK_MAX)
	{
		w.h.window = i;
		h_eq_i64("ajout", tl_apply(&l, WMS_WIN_ADD, &w), 1);
		i++;
	}
	w.h.window = TASK_MAX + 1;
	h_eq_i64("au dela du plafond", tl_apply(&l, WMS_WIN_ADD, &w), 0);
	h_eq_u64("plafond", l.count, TASK_MAX);
	w.h.window = 7;
	tl_apply(&l, WMS_WIN_DEL, &w);
	w.h.window = TASK_MAX + 1;
	h_eq_i64("place liberee", tl_apply(&l, WMS_WIN_ADD, &w), 1);
	w.h.window = TASK_MAX;
	h_eq_i64("derniere suppression", tl_apply(&l, WMS_WIN_DEL, &w), 1);
}

int	main(void)
{
	h_begin("a20/tasklist-state");
	h_run("taches: activation exclusive", state_activation_exclusive);
	h_run("taches: mise a jour et masquage", state_mise_a_jour_et_masquage);
	h_run("taches: titres hostiles", state_titres_hostiles);
	h_run("taches: plafond de fenetres", state_plafond_de_fenetres);
	return (h_end());
}
