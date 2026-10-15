#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static t_ctl	*mkcheck(t_fakeui *u)
{
	t_ctl	*c;

	c = fake_add(u, fake_spec(CT_CHECK, 1, rect_make(10, 10, 100, 14),
				"&Gras"));
	c->cb = fake_cb;
	return (c);
}

static void	toggle_by_mouse(void)
{
	t_fakeui	u;
	t_ctl		*c;

	fake_begin(&u, 200, 100);
	c = mkcheck(&u);
	h_true(!(c->flags & CTL_CHECKED), "decochee au depart");
	fake_click(&u.r, 12, 12);
	h_true((c->flags & CTL_CHECKED) != 0, "cochee apres un clic");
	fake_click(&u.r, 90, 12);
	h_true(!(c->flags & CTL_CHECKED), "clic libelle : decochee");
	h_eq_i64("deux notifications", fake_cmd_find(1, CN_CLICKED), 2);
	h_eq_i64("deux callbacks", fake_cmd_cbs(), 2);
	fake_down(&u.r, 12, 12);
	fake_move(&u.r, 150, 60);
	fake_up(&u.r, 150, 60);
	h_true(!(c->flags & CTL_CHECKED), "relache dehors : pas de bascule");
	fake_done(&u);
}

static void	toggle_by_keyboard(void)
{
	t_fakeui	u;
	t_ctl		*c;

	fake_begin(&u, 200, 100);
	c = mkcheck(&u);
	ctl_focus(&u.r, c);
	h_true(fake_press(&u.r, VK_SPACE, 0), "Espace consomme");
	h_true((c->flags & CTL_CHECKED) != 0, "cochee");
	h_true(fake_press(&u.r, VK_SPACE, INPM_REPEAT), "repetition consommee");
	h_true((c->flags & CTL_CHECKED) != 0, "repetition sans effet");
	h_true(!fake_press(&u.r, VK_RETURN, 0), "Entree non consommee");
	fake_press(&u.r, 'G', INPM_ALT);
	h_true(!(c->flags & CTL_CHECKED), "Alt+G : decochee");
	fake_done(&u);
}

static void	disabled_check(void)
{
	t_fakeui	u;
	t_ctl		*c;

	fake_begin(&u, 200, 100);
	c = mkcheck(&u);
	ctl_set_flag(&u.r, c, CTL_ENABLED, false);
	fake_click(&u.r, 12, 12);
	ctl_focus(&u.r, c);
	fake_press(&u.r, VK_SPACE, 0);
	fake_press(&u.r, 'G', INPM_ALT);
	h_true(!(c->flags & CTL_CHECKED), "desactivee : jamais cochee");
	h_eq_i64("aucune notification", fake_cmd_count(), 0);
	ctl_set_flag(&u.r, c, CTL_CHECKED, true);
	h_true((c->flags & CTL_CHECKED) != 0, "coche par programme");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/check");
	h_run("case : souris", toggle_by_mouse);
	h_run("case : clavier, repetition, mnemonique", toggle_by_keyboard);
	h_run("case desactivee", disabled_check);
	return (h_end());
}
