#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	set_clamps(void)
{
	t_fakeui	u;
	t_ctl		*s;

	fake_begin(&u, 200, 120);
	s = fake_bar(&u, 90, 10);
	ctl_scroll_move(&u.r, s, 500);
	h_eq_i64("pos plafonnee", ctl_scroll_pos(s), 90);
	ctl_scroll_set(&u.r, s, 50, 10);
	h_eq_i64("pos ramenee par un max plus petit", ctl_scroll_pos(s), 50);
	ctl_scroll_set(&u.r, s, -5, 0);
	h_eq_i64("max negatif : 0", ((t_scrst *)s->priv)->max, 0);
	h_eq_i64("page 0 : 1", ((t_scrst *)s->priv)->page, 1);
	h_eq_i64("pos a 0", ctl_scroll_pos(s), 0);
	ctl_scroll_set(&u.r, s, 2000000000, 2000000000);
	h_eq_i64("max plafonne", ((t_scrst *)s->priv)->max, CTL_SCROLL_MAX);
	ctl_scroll_move(&u.r, s, -3);
	h_eq_i64("pos negative : 0", ctl_scroll_pos(s), 0);
	h_eq_i64("pas de notification par programme", fake_cmd_count(), 0);
	fake_done(&u);
}

static void	keys(void)
{
	t_fakeui	u;
	t_ctl		*s;

	fake_begin(&u, 200, 120);
	s = fake_bar(&u, 90, 10);
	ctl_focus(&u.r, s);
	fake_press(&u.r, VK_DOWN, 0);
	fake_press(&u.r, VK_RIGHT, 0);
	h_eq_i64("Bas, Droite", ctl_scroll_pos(s), 2);
	fake_press(&u.r, VK_NEXT, 0);
	h_eq_i64("PgDn", ctl_scroll_pos(s), 12);
	fake_press(&u.r, VK_UP, 0);
	fake_press(&u.r, VK_PRIOR, 0);
	h_eq_i64("Haut, PgUp", ctl_scroll_pos(s), 1);
	fake_press(&u.r, VK_END, 0);
	h_eq_i64("Fin", ctl_scroll_pos(s), 90);
	h_true(fake_press(&u.r, VK_DOWN, 0), "au bord : consomme");
	h_eq_i64("notifications : 6 changements", fake_cmd_find(1, CN_SCROLL), 6);
	fake_press(&u.r, VK_HOME, 0);
	h_eq_i64("Debut", ctl_scroll_pos(s), 0);
	h_true(!fake_press(&u.r, VK_DOWN, INPM_CTRL), "Ctrl non consomme");
	fake_done(&u);
}

static void	mouse_parts(void)
{
	t_fakeui	u;
	t_ctl		*s;

	fake_begin(&u, 200, 120);
	s = fake_bar(&u, 90, 10);
	ctl_scroll_move(&u.r, s, 45);
	fake_click(&u.r, 105, 15);
	h_eq_i64("fleche haut", ctl_scroll_pos(s), 44);
	fake_click(&u.r, 105, 100);
	h_eq_i64("fleche bas", ctl_scroll_pos(s), 45);
	fake_click(&u.r, 105, 35);
	h_eq_i64("piste avant le pouce : page precedente", ctl_scroll_pos(s), 35);
	fake_click(&u.r, 105, 75);
	h_eq_i64("piste apres le pouce : page suivante", ctl_scroll_pos(s), 45);
	fake_wheel(&u.r, 105, 60, 1);
	h_eq_i64("molette", ctl_scroll_pos(s), 42);
	h_eq_i64("notifications", fake_cmd_find(1, CN_SCROLL), 5);
	fake_done(&u);
}

static void	thumb_drag(void)
{
	t_fakeui	u;
	t_ctl		*s;

	fake_begin(&u, 200, 120);
	s = fake_bar(&u, 90, 10);
	ctl_scroll_move(&u.r, s, 45);
	fake_down(&u.r, 105, 59);
	h_true(u.r.capture == s, "capture sur le pouce");
	fake_move(&u.r, 105, 59);
	h_eq_i64("sans mouvement : inchange", ctl_scroll_pos(s), 45);
	fake_move(&u.r, 105, 200);
	h_eq_i64("tout en bas", ctl_scroll_pos(s), 90);
	fake_move(&u.r, 105, -50);
	h_eq_i64("tout en haut", ctl_scroll_pos(s), 0);
	fake_up(&u.r, 105, 60);
	fake_move(&u.r, 105, 100);
	h_eq_i64("apres relache : plus de glisse", ctl_scroll_pos(s), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/scroll");
	h_run("ctl_scroll_set : bornes et plafonds", set_clamps);
	h_run("clavier : fleches, pages, Debut, Fin", keys);
	h_run("souris : fleches, pistes, molette", mouse_parts);
	h_run("glisse du pouce", thumb_drag);
	return (h_end());
}
