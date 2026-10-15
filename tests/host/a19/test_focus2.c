#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	click_gives_focus(void)
{
	t_fakeui	u;
	t_ctl		*b;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	b = fk_c(&u, CT_BUTTON, 1, rect_make(10, 10, 60, 20));
	l = fk_c(&u, CT_LABEL, 2, rect_make(10, 40, 60, 12));
	fake_click(&u.r, 15, 45);
	h_true(u.r.focus == NULL, "un libelle ne prend pas le focus");
	fake_click(&u.r, 15, 15);
	h_true(u.r.focus == b, "un bouton prend le focus au clic");
	fake_click(&u.r, 15, 45);
	h_true(u.r.focus == b && l != NULL, "clic sur un libelle : focus garde");
	ctl_focus(&u.r, NULL);
	h_true(u.r.focus == NULL, "ctl_focus(NULL) retire le focus");
	fake_done(&u);
}

static void	tab_selects_edit_text(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 100);
	e = fake_add(&u, fake_spec(CT_EDIT, 1, rect_make(10, 10, 100, 20), "abc"));
	fk_c(&u, CT_BUTTON, 2, rect_make(10, 40, 60, 20));
	ed = e->priv;
	ctl_focus(&u.r, e);
	h_true(ed->anchor == ed->caret, "ctl_focus : pas de selection");
	fake_press(&u.r, VK_TAB, 0);
	fake_press(&u.r, VK_TAB, 0);
	h_true(u.r.focus == e, "retour sur l'edit");
	h_true(ed->anchor == 0 && ed->caret == 3, "Tab selectionne tout le texte");
	fake_type(&u.r, "z");
	h_eq_str("la frappe remplace la selection", e->text, "z");
	fake_done(&u);
}

static void	focus_dropped(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	fk_c(&u, CT_BUTTON, 1, rect_make(10, 10, 60, 20));
	b = fk_c(&u, CT_BUTTON, 2, rect_make(10, 40, 60, 20));
	ctl_focus(&u.r, b);
	ctl_set_flag(&u.r, b, CTL_VISIBLE, false);
	h_true(u.r.focus == NULL, "masquage : focus perdu");
	ctl_set_flag(&u.r, b, CTL_VISIBLE, true);
	ctl_focus(&u.r, b);
	ctl_set_flag(&u.r, b, CTL_ENABLED, false);
	h_true(u.r.focus == NULL, "desactivation : focus perdu");
	ctl_set_flag(&u.r, b, CTL_ENABLED, true);
	ctl_focus(&u.r, b);
	ctl_remove(&u.r, b);
	h_true(u.r.focus == NULL, "retrait : focus perdu");
	fake_press(&u.r, VK_TAB, 0);
	h_eq_i64("Tab reprend au premier", u.r.focus->id, 1);
	fake_done(&u);
}

static void	no_stops_and_misc(void)
{
	t_fakeui	u;
	t_fakeui	o;
	t_ctl		*x;

	fake_begin(&u, 200, 100);
	fk_c(&u, CT_LABEL, 1, rect_make(10, 10, 60, 12));
	h_true(!fake_press(&u.r, VK_TAB, 0), "aucun arret : Tab non consomme");
	h_true(!fake_press(&u.r, VK_TAB, INPM_SHIFT), "idem Maj+Tab");
	fk_c(&u, CT_BUTTON, 2, rect_make(10, 40, 60, 20));
	h_true(!fake_press(&u.r, VK_TAB, INPM_CTRL), "Ctrl+Tab non consomme");
	h_true(!fake_press(&u.r, VK_TAB, INPM_ALT), "Alt+Tab non consomme");
	fake_ui_open(&o, 50, 50);
	x = fk_c(&o, CT_BUTTON, 1, rect_make(0, 0, 20, 20));
	ctl_focus(&u.r, x);
	h_true(u.r.focus == NULL, "focus sur un controle etranger refuse");
	fake_ui_close(&o);
	h_true(fake_press(&u.r, VK_TAB, 0), "un seul arret : Tab consomme");
	h_true(fake_press(&u.r, VK_TAB, 0), "et il boucle sur lui-meme");
	h_eq_i64("toujours le meme", u.r.focus->id, 2);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/focus2");
	h_run("clic : focus selon le type", click_gives_focus);
	h_run("Tab selectionne le texte d'un edit", tab_selects_edit_text);
	h_run("focus perdu : masque, desactive, retire", focus_dropped);
	h_run("aucun arret, un arret, controle etranger", no_stops_and_misc);
	return (h_end());
}
