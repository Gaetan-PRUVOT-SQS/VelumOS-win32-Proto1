#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	size_and_clamp(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_rect		r;

	fake_begin(&u, 300, 250);
	m = fake_menu(&u, 20, 20);
	r = fake_mlevel(m, 0);
	h_true(fake_rect_eq(r, rect_make(20, 20, 108, 170)), "taille et position");
	h_true(fake_rect_eq(m->rect, r), "le controle englobe le niveau");
	ctl_menu_close(&u.r, m);
	m = fake_menu(&u, 290, 240);
	r = fake_mlevel(m, 0);
	h_true(r.x + r.w == 300 && r.y + r.h == 250, "ramene dans la surface");
	ctl_menu_close(&u.r, m);
	m = fake_menu(&u, -50, -40);
	r = fake_mlevel(m, 0);
	h_true(r.x == 0 && r.y == 0, "coordonnees negatives : bord haut gauche");
	fake_done(&u);
}

static void	submenu_placement(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 300, 250);
	m = fake_menu(&u, 20, 20);
	fake_keys(&u.r, VK_DOWN, 0, 6);
	fake_press(&u.r, VK_RIGHT, 0);
	h_true(fake_rect_eq(fake_mlevel(m, 1), rect_make(126, 20 + 2 + 5 * 22 + 6
				- 2, 72, 2 * 2 + 3 * 22 + 6)), "a droite du parent");
	h_true(fake_rect_eq(m->rect, rect_union(fake_mlevel(m, 0),
				fake_mlevel(m, 1))), "enveloppe des deux niveaux");
	ctl_menu_close(&u.r, m);
	m = fake_menu(&u, 200, 20);
	fake_keys(&u.r, VK_DOWN, 0, 6);
	fake_press(&u.r, VK_RIGHT, 0);
	h_eq_i64("pas de place a droite : a gauche", fake_mlevel(m, 1).x,
		fake_mlevel(m, 0).x - 72 + 2);
	fake_done(&u);
}

static void	dirty_exact(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_rect		want[2];

	fake_begin(&u, 300, 250);
	m = fake_menu(&u, 20, 20);
	ctl_paint(&u.r);
	fk_mv(&u, m, 0, 0);
	want[0] = fake_mrect(m, 0, 0);
	h_true(fake_dirty_is(&u.r, want, 1), "survol : rect de l'element");
	ctl_paint(&u.r);
	fk_mv(&u, m, 0, 1);
	want[1] = fake_mrect(m, 0, 1);
	h_true(fake_dirty_is(&u.r, want, 2), "ancien et nouveau elements");
	ctl_paint(&u.r);
	fk_mv(&u, m, 0, 1);
	h_true(fake_dirty_is(&u.r, NULL, 0), "meme element : rien");
	fk_mv(&u, m, 0, 6);
	h_true(fake_dirty_covers(&u.r, fake_mlevel(m, 1)), "sous-menu sale");
	ctl_paint(&u.r);
	fake_press(&u.r, VK_ESCAPE, 0);
	fake_press(&u.r, VK_ESCAPE, 0);
	h_true(fake_dirty_covers(&u.r, rect_make(20, 20, 108, 170)), "ferme");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_geom");
	h_run("taille, ramene dans la surface", size_and_clamp);
	h_run("sous-menu : droite, ou gauche sans place", submenu_placement);
	h_run("zones sales du menu", dirty_exact);
	return (h_end());
}
