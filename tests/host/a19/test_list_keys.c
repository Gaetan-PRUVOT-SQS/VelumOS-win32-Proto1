#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	arrows_and_limits(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 20);
	fake_press(&u.r, VK_DOWN, 0);
	h_eq_i64("Bas depuis rien : premier", ctl_list_selected(l), 0);
	fake_keys(&u.r, VK_DOWN, 0, 3);
	h_eq_i64("Bas x3", ctl_list_selected(l), 3);
	fake_keys(&u.r, VK_UP, 0, 10);
	h_eq_i64("Haut au bord : reste a 0", ctl_list_selected(l), 0);
	fake_press(&u.r, VK_END, 0);
	h_eq_i64("Fin", ctl_list_selected(l), 19);
	fake_press(&u.r, VK_DOWN, 0);
	h_eq_i64("Bas au bord : reste a 19", ctl_list_selected(l), 19);
	h_eq_i64("une notification par changement", fake_cmd_find(1, CN_SELECT), 8);
	h_eq_i64("selection toujours visible", ((t_list *)l->priv)->sb.pos, 17);
	fake_press(&u.r, VK_UP, 0);
	h_eq_i64("remonte d'un cran sans defiler", ((t_list *)l->priv)->sb.pos, 17);
	fake_done(&u);
}

static void	pages_and_enter(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 20);
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree sans selection : consomme");
	h_eq_i64("pas de CN_ENTER sans selection", fake_cmd_find(1, CN_ENTER), 0);
	fake_press(&u.r, VK_NEXT, 0);
	h_eq_i64("PgDn depuis rien : premier", ctl_list_selected(l), 0);
	fake_press(&u.r, VK_NEXT, 0);
	h_eq_i64("PgDn : 3 lignes", ctl_list_selected(l), 3);
	fake_press(&u.r, VK_PRIOR, 0);
	h_eq_i64("PgUp : 3 lignes", ctl_list_selected(l), 0);
	fake_press(&u.r, VK_PRIOR, 0);
	h_eq_i64("PgUp au bord", ctl_list_selected(l), 0);
	fake_press(&u.r, VK_RETURN, 0);
	h_eq_i64("CN_ENTER avec selection", fake_cmd_find(1, CN_ENTER), 1);
	fake_keys(&u.r, VK_NEXT, 0, 20);
	h_eq_i64("PgDn au bord", ctl_list_selected(l), 19);
	fake_done(&u);
}

static void	modifiers_and_disabled(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 5);
	h_true(!fake_press(&u.r, VK_DOWN, INPM_CTRL), "Ctrl+Bas non consomme");
	h_true(!fake_press(&u.r, VK_DOWN, INPM_ALT), "Alt+Bas non consomme");
	h_true(!fake_press(&u.r, VK_LEFT, 0), "Gauche non consomme");
	h_true(!fake_press(&u.r, 'A', 0), "lettre non consommee");
	h_eq_i64("aucune selection", ctl_list_selected(l), -1);
	ctl_set_flag(&u.r, l, CTL_ENABLED, false);
	h_true(!fake_press(&u.r, VK_DOWN, 0), "desactivee : non consomme");
	h_eq_i64("toujours rien", ctl_list_selected(l), -1);
	h_eq_i64("pas de notification", fake_cmd_count(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/list_keys");
	h_run("fleches, Debut, Fin, bords", arrows_and_limits);
	h_run("pages et Entree", pages_and_enter);
	h_run("modificateurs et liste desactivee", modifiers_and_disabled);
	return (h_end());
}
