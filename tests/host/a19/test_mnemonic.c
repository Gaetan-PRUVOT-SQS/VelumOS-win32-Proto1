#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	scene(t_fakeui *u)
{
	t_ctl	*c;

	fake_add(u, fake_spec(CT_LABEL, 1, rect_make(0, 0, 40, 12), "&Nom"));
	fk_c(u, CT_EDIT, 2, rect_make(50, 0, 60, 20));
	c = fake_add(u, fake_spec(CT_BUTTON, 3, rect_make(0, 30, 40, 20), "&OK"));
	c->cb = fake_cb;
	fake_add(u, fake_spec(CT_CHECK, 4, rect_make(0, 60, 60, 14), "&Gras"));
	fake_add(u, fake_spec(CT_RADIO, 5, rect_make(0, 80, 40, 14), "&A"));
	fake_add(u, fake_spec(CT_RADIO, 6, rect_make(0, 100, 40, 14), "&B"));
	fake_add(u, fake_spec(CT_BUTTON, 7, rect_make(0, 120, 40, 20), "&Z"));
	fake_add(u, fake_spec(CT_BUTTON, 8, rect_make(50, 30, 40, 20), "&H"));
	fake_add(u, fake_spec(CT_BUTTON, 9, rect_make(50, 120, 40, 20), "&Quit"));
	fake_add(u, fake_spec(CT_BUTTON, 10, rect_make(100, 120, 40, 20), "&Qui"));
	ctl_set_flag(&u->r, ctl_find(&u->r, 7), CTL_ENABLED, false);
	ctl_set_flag(&u->r, ctl_find(&u->r, 8), CTL_VISIBLE, false);
}

static void	activations(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 150);
	scene(&u);
	h_true(fake_press(&u.r, 'N', INPM_ALT), "Alt+N consomme");
	h_true(u.r.focus == ctl_find(&u.r, 2), "libelle : focus sur l'edit");
	h_true(fake_press(&u.r, 'O', INPM_ALT), "Alt+O consomme");
	h_eq_i64("bouton clique", fake_cmd_find(3, CN_CLICKED), 1);
	h_true(u.r.focus == ctl_find(&u.r, 3), "le bouton prend le focus");
	fake_press(&u.r, 'g', INPM_ALT);
	h_true(ctl_find(&u.r, 4)->flags & CTL_CHECKED, "Alt+g (minuscule) : case");
	fake_press(&u.r, 'B', INPM_ALT);
	h_true(ctl_find(&u.r, 6)->flags & CTL_CHECKED, "Alt+B : radio B");
	h_true(u.r.focus == ctl_find(&u.r, 6), "radio : focus");
	fake_done(&u);
}

static void	refusals(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 150);
	scene(&u);
	h_true(!fake_press(&u.r, 'Z', INPM_ALT), "desactive : non consomme");
	h_true(!fake_press(&u.r, 'H', INPM_ALT), "masque : non consomme");
	h_true(!fake_press(&u.r, 'X', INPM_ALT), "sans correspondance");
	h_true(!fake_press(&u.r, 'O', 0), "sans Alt : non consomme");
	h_true(!fake_press(&u.r, 'O', INPM_ALT | INPM_CTRL), "AltGr n'est pas Alt");
	h_true(!fake_press(&u.r, 'O', INPM_CTRL), "Ctrl seul");
	h_eq_i64("aucune activation", fake_cmd_count(), 0);
	fake_done(&u);
}

static void	duplicates_cycle(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 150);
	scene(&u);
	fake_press(&u.r, 'Q', INPM_ALT);
	h_eq_i64("premier Q", u.r.focus->id, 9);
	fake_press(&u.r, 'Q', INPM_ALT);
	h_eq_i64("deuxieme Q", u.r.focus->id, 10);
	fake_press(&u.r, 'Q', INPM_ALT);
	h_eq_i64("retour au premier", u.r.focus->id, 9);
	ctl_set_text(&u.r, ctl_find(&u.r, 9), "&&Quit");
	ctl_set_text(&u.r, ctl_find(&u.r, 10), "Qui&&");
	h_true(!fake_press(&u.r, 'Q', INPM_ALT), "&& n'est pas un mnemonique");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/mnemonic");
	h_run("Alt+lettre : libelle, bouton, case, radio", activations);
	h_run("refus : desactive, masque, AltGr, sans Alt", refusals);
	h_run("doublons : parcours cyclique, &&", duplicates_cycle);
	return (h_end());
}
