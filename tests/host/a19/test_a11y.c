#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static const int	g_order[8] = {3, 4, 5, 8, 10, 11, 1, 2};

static void	tab_cycle(void)
{
	t_fakeui	u;
	int			got[8];
	int			i;

	fake_begin(&u, 320, 260);
	fake_dialog(&u);
	h_eq_i64("8 arrets", fake_tabs(&u, got, true), 8);
	i = 0;
	while (i < 8)
	{
		h_eq_i64("ordre logique (arbre)", got[i], g_order[i]);
		i++;
	}
	fake_press(&u.r, VK_TAB, 0);
	h_eq_i64("pas de piege : retour au premier", u.r.focus->id, 3);
	fake_press(&u.r, VK_TAB, INPM_SHIFT);
	h_eq_i64("Maj+Tab : retour au dernier", u.r.focus->id, 2);
	fake_done(&u);
}

static void	focus_always_visible(void)
{
	t_fakeui	u;
	int			i;

	fake_begin(&u, 320, 260);
	fake_dialog(&u);
	i = 0;
	while (i < 8)
	{
		fake_press(&u.r, VK_TAB, 0);
		h_eq_i64("le focus suit l'ordre", u.r.focus->id, g_order[i]);
		h_true(fake_focus_visible(&u, u.r.focus), "focus dessine");
		i++;
	}
	ctl_focus(&u.r, NULL);
	h_true(!fake_focus_visible(&u, ctl_find(&u.r, 1)), "sans focus : aucun");
	fake_done(&u);
}

static void	keyboard_only(void)
{
	t_fakeui	u;

	fake_begin(&u, 320, 260);
	fake_dialog(&u);
	fake_press(&u.r, 'N', INPM_ALT);
	h_eq_i64("Alt+N : champ du nom", u.r.focus->id, 3);
	fake_type(&u.r, "Zoe");
	fake_press(&u.r, VK_TAB, 0);
	fake_type(&u.r, "mot");
	fake_press(&u.r, VK_TAB, 0);
	fake_press(&u.r, VK_SPACE, 0);
	h_true(ctl_find(&u.r, 5)->flags & CTL_CHECKED, "case cochee a l'Espace");
	fake_press(&u.r, VK_TAB, 0);
	fake_press(&u.r, VK_DOWN, 0);
	h_true(ctl_find(&u.r, 9)->flags & CTL_CHECKED, "radio C par fleche");
	fake_press(&u.r, VK_TAB, 0);
	fake_press(&u.r, VK_DOWN, 0);
	h_eq_i64("liste : premier", ctl_list_selected(ctl_find(&u.r, 10)), 0);
	fake_press(&u.r, VK_RETURN, 0);
	h_eq_i64("liste : Entree", fake_cmd_find(10, CN_ENTER), 1);
	fake_press(&u.r, VK_ESCAPE, 0);
	h_eq_i64("Echap : Annuler", fake_cmd_find(CTL_ID_CANCEL, CN_CLICKED), 1);
	fake_done(&u);
}

static void	stable_identity(void)
{
	t_fakeui	u;
	t_ctl		*ok;
	t_ctl		*extra;

	fake_begin(&u, 320, 260);
	fake_dialog(&u);
	ok = ctl_find(&u.r, CTL_ID_OK);
	extra = fk_c(&u, CT_LABEL, 99, rect_make(0, 0, 5, 5));
	ctl_remove(&u.r, extra);
	ctl_paint(&u.r);
	h_true(ctl_find(&u.r, CTL_ID_OK) == ok, "identifiant stable");
	h_eq_str("nom accessible conserve", ok->text, "OK");
	h_eq_str("nom du retour", ctl_find(&u.r, 22)->text, "&Secret");
	h_true(ok->flags & CTL_DEFAULT, "role : bouton par defaut");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/a11y");
	h_run("Tab : cycle complet sans piege, ordre logique", tab_cycle);
	h_run("focus visible sur chaque arret", focus_always_visible);
	h_run("dialogue entier au clavier seul", keyboard_only);
	h_run("identifiants et noms stables", stable_identity);
	return (h_end());
}
