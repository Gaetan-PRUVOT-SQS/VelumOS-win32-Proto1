#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	click_select(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 2);
	fake_click(&u.r, 20, 7 + 15 + 3);
	h_eq_i64("deuxieme ligne", ctl_list_selected(l), 1);
	h_true(u.r.focus == l, "le clic donne le focus");
	h_eq_i64("une notification", fake_cmd_find(1, CN_SELECT), 1);
	fake_click(&u.r, 20, 7 + 15);
	h_eq_i64("meme ligne : silence", fake_cmd_find(1, CN_SELECT), 1);
	fake_click(&u.r, 20, 7 + 30 + 5);
	h_eq_i64("sous la derniere ligne : inchange", ctl_list_selected(l), 1);
	fake_click(&u.r, 6, 20);
	h_eq_i64("sur le cadre : inchange", ctl_list_selected(l), 1);
	fake_click(&u.r, 20, 10);
	h_eq_i64("premiere ligne", ctl_list_selected(l), 0);
	fake_done(&u);
}

static void	wheel_scroll(void)
{
	t_fakeui	u;
	t_ctl		*l;
	t_list		*li;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 20);
	li = l->priv;
	ctl_list_select(&u.r, l, 10);
	h_eq_i64("selection visible", li->sb.pos, 8);
	fake_wheel(&u.r, 20, 20, 1);
	h_eq_i64("molette vers le haut", li->sb.pos, 5);
	fake_wheel(&u.r, 20, 20, -1);
	h_eq_i64("molette vers le bas", li->sb.pos, 8);
	fake_wheel(&u.r, 20, 20, -100);
	h_eq_i64("maximum", li->sb.pos, 17);
	fake_wheel(&u.r, 20, 20, -1);
	h_eq_i64("au maximum : inchange", fake_cmd_find(1, CN_SCROLL), 3);
	fake_wheel(&u.r, 20, 20, 2000000000);
	h_eq_i64("molette enorme : minimum", li->sb.pos, 0);
	h_eq_i64("la selection ne bouge pas", ctl_list_selected(l), 10);
	fake_done(&u);
}

static void	double_click(void)
{
	t_fakeui	u;

	fake_begin(&u, 200, 100);
	fake_list(&u, 5);
	ctl_set_clock(&u.r, fake_clock_now);
	fake_click(&u.r, 20, 10);
	fake_clock_set(100000000);
	fake_click(&u.r, 20, 10);
	h_eq_i64("double clic", fake_cmd_find(1, CN_DBLCLK), 1);
	h_eq_i64("une seule selection", fake_cmd_find(1, CN_SELECT), 1);
	fake_clock_set(150000000);
	fake_click(&u.r, 20, 10);
	h_eq_i64("le troisieme clic n'est pas un double", fake_cmd_find(1,
			CN_DBLCLK), 1);
	fake_clock_set(800000000);
	fake_click(&u.r, 20, 10);
	fake_clock_set(1400000000);
	fake_click(&u.r, 20, 10);
	h_eq_i64("trop lent : pas de double", fake_cmd_find(1, CN_DBLCLK), 1);
	fake_clock_set(1500000000);
	fake_click(&u.r, 20, 25);
	h_eq_i64("autre ligne : pas de double", fake_cmd_find(1, CN_DBLCLK), 1);
	fake_done(&u);
}

static void	scrollbar_parts(void)
{
	t_fakeui	u;
	t_ctl		*l;
	t_list		*li;

	fake_begin(&u, 200, 100);
	l = fake_list(&u, 20);
	li = l->priv;
	fake_click(&u.r, 90, 55);
	h_eq_i64("fleche bas", li->sb.pos, 1);
	fake_click(&u.r, 90, 42);
	h_eq_i64("piste sous le pouce : page", li->sb.pos, 4);
	fake_click(&u.r, 90, 10);
	h_eq_i64("fleche haut", li->sb.pos, 3);
	h_eq_i64("la selection n'a pas bouge", ctl_list_selected(l), -1);
	fake_move(&u.r, 90, 10);
	fake_down(&u.r, 90, 12);
	h_true(u.r.capture == l, "capture sur la barre");
	fake_up(&u.r, 90, 12);
	h_true(u.r.capture == NULL, "relache");
	h_true(fake_cmd_find(1, CN_SCROLL) >= 3, "notifications de defilement");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/list_mouse");
	h_run("clic : selection, bords, focus", click_select);
	h_run("molette : bornes, notifications", wheel_scroll);
	h_run("double clic : delai, ligne, remise a zero", double_click);
	h_run("barre : fleches, page, capture", scrollbar_parts);
	return (h_end());
}
