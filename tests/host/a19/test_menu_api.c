#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	invalid_specs(void)
{
	t_fakeui		u;
	t_ctlmenuitem	a[70];
	t_ctlmenuspec	ms;
	int				live;

	fake_begin(&u, 200, 150);
	live = fake_mem_live();
	memset(a, 0, sizeof(a));
	ms.id = 100;
	ms.at = (t_point){10, 10};
	ms.items = a;
	ms.count = 0;
	h_true(ctl_menu_popup(&u.r, &ms) == NULL, "0 element");
	ms.count = CTL_MENU_ITEMS_MAX + 1;
	h_true(ctl_menu_popup(&u.r, &ms) == NULL, "65 elements");
	ms.items = NULL;
	ms.count = 1;
	h_true(ctl_menu_popup(&u.r, &ms) == NULL, "items NULL");
	ms.items = a;
	ms.at = (t_point){CTL_COORD_MAX + 1, 0};
	h_true(ctl_menu_popup(&u.r, &ms) == NULL, "position hors bornes");
	h_eq_i64("rien alloue", fake_mem_live(), live);
	fake_done(&u);
}

static void	depth_limits(void)
{
	t_fakeui		u;
	t_ctlmenuitem	a[6];
	t_ctl			*m;

	fake_begin(&u, 200, 150);
	m = fake_chain(&u, a, 4);
	h_true(m != NULL, "4 niveaux acceptes");
	ctl_menu_close(&u.r, m);
	h_true(fake_chain(&u, a, 5) == NULL, "5 niveaux refuses");
	h_true(ctl_find(&u.r, 100) == NULL, "rien d'ajoute");
	a[0].nsub = CTL_MENU_ITEMS_MAX + 1;
	h_true(fake_chain(&u, a, 2) != NULL, "refait la chaine");
	fake_done(&u);
}

static void	copy_semantics(void)
{
	t_fakeui		u;
	t_ctlmenuitem	*it;
	t_ctlmenuspec	ms;
	t_ctl			*m;

	fake_begin(&u, 200, 150);
	it = calloc(2, sizeof(*it));
	fake_item(&it[0], strdup("&Copie"), 7, 0);
	fake_item(&it[1], "", 0, CTL_MI_SEPARATOR);
	ms.id = 100;
	ms.at = (t_point){10, 10};
	ms.items = it;
	ms.count = 2;
	m = ctl_menu_popup(&u.r, &ms);
	memset((void *)it[0].text, 'Z', 6);
	free((void *)it[0].text);
	memset(it, 0xaa, 2 * sizeof(*it));
	free(it);
	ctl_paint(&u.r);
	h_eq_str("texte copie", fake_log_last(FK_TEXT)->text, "Copie");
	fake_press(&u.r, VK_DOWN, 0);
	fake_press(&u.r, VK_RETURN, 0);
	h_true(m != NULL && g_fake_picked == -1, "pas de cb : pas de resultat");
	fake_done(&u);
}

static void	flags_and_leaf_sub(void)
{
	t_fakeui		u;
	t_ctlmenuitem	a[2];
	t_ctlmenuspec	ms;
	t_ctl			*m;

	fake_begin(&u, 200, 150);
	fake_item(&a[0], "&Un", 5, 0x80);
	a[0].sub = &a[1];
	a[0].nsub = 0;
	ms.id = 100;
	ms.at = (t_point){10, 10};
	ms.items = a;
	ms.count = 1;
	m = ctl_menu_popup(&u.r, &ms);
	m->cb = fake_cb_pick;
	fake_char(&u.r, 'u');
	h_eq_i64("drapeau inconnu ignore, feuille", g_fake_picked, 5);
	h_eq_i64("resultat avant choix", ctl_menu_result(NULL), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_api");
	h_run("specs invalides : refus sans effet de bord", invalid_specs);
	h_run("niveaux maximum", depth_limits);
	h_run("copie des elements : l'appelant peut tout liberer", copy_semantics);
	h_run("drapeaux inconnus, sub vide", flags_and_leaf_sub);
	return (h_end());
}
