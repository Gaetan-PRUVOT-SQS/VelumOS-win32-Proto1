#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	keys_activate(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	ctl_focus(&u.r, b);
	h_true(fake_press(&u.r, VK_SPACE, 0), "Espace consomme");
	h_true(fake_press(&u.r, VK_RETURN, 0), "Entree consomme");
	h_eq_i64("deux clics", fake_cmd_find(1, CN_CLICKED), 2);
	h_true(fake_press(&u.r, VK_SPACE, INPM_REPEAT), "repetition consommee");
	h_eq_i64("repetition sans clic", fake_cmd_find(1, CN_CLICKED), 2);
	h_true(!fake_press(&u.r, VK_SPACE, INPM_CTRL), "Ctrl+Espace non consomme");
	h_true(!fake_press(&u.r, VK_SPACE, INPM_ALT), "Alt+Espace non consomme");
	h_true(!fake_press(&u.r, VK_LEFT, 0), "autre touche non consommee");
	h_eq_i64("toujours deux clics", fake_cmd_find(1, CN_CLICKED), 2);
	fake_done(&u);
}

static void	disabled_button(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	ctl_set_flag(&u.r, b, CTL_ENABLED, false);
	h_true(fake_down(&u.r, 15, 15), "appui sur desactive : avale");
	h_true(u.r.capture == NULL && u.r.focus == NULL, "ni capture ni focus");
	fake_up(&u.r, 15, 15);
	ctl_focus(&u.r, b);
	h_true(u.r.focus == NULL, "focus refuse");
	h_true(!fake_press(&u.r, VK_SPACE, 0), "clavier sans effet");
	h_eq_i64("aucune notification", fake_cmd_count(), 0);
	fake_done(&u);
}

static void	disabled_or_hidden_parent(void)
{
	t_fakeui	u;
	t_ctl		*p;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	p = fake_add(&u, fake_spec(CT_PANEL, 5, rect_make(0, 0, 100, 50), ""));
	b = fake_addp(&u, p, fake_spec(CT_BUTTON, 1, rect_make(10, 10, 60, 20),
				""));
	ctl_focus(&u.r, b);
	h_true(u.r.focus == b, "focus dans un panneau actif");
	ctl_set_flag(&u.r, p, CTL_ENABLED, false);
	h_true(u.r.focus == NULL, "panneau desactive : focus perdu");
	fake_click(&u.r, 15, 15);
	h_eq_i64("clic sans effet", fake_cmd_count(), 0);
	ctl_set_flag(&u.r, p, CTL_ENABLED, true);
	ctl_set_flag(&u.r, p, CTL_VISIBLE, false);
	fake_click(&u.r, 15, 15);
	h_eq_i64("panneau masque : clic sans effet", fake_cmd_count(), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/button_kbd");
	h_run("Espace, Entree, repetition, modificateurs", keys_activate);
	h_run("bouton desactive", disabled_button);
	h_run("panneau parent desactive ou masque", disabled_or_hidden_parent);
	return (h_end());
}
