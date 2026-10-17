#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	panels_and_hot(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 300, 250);
	m = fake_menu(&u, 20, 20);
	ctl_paint(&u.r);
	h_eq_i64("un panneau", fake_log_kind(FK_MENU_PANEL), 1);
	h_eq_i64("rien de survole", fake_log_kind(FK_MENU_ITEM), 0);
	fake_log_clear();
	fake_press(&u.r, VK_DOWN, 0);
	ctl_paint(&u.r);
	h_eq_i64("element actif", fake_log_kind(FK_MENU_ITEM), 1);
	h_eq_i64("etat LS_HOT", fake_log_last(FK_MENU_ITEM)->a, LS_HOT);
	h_true(fake_rect_eq(fake_log_last(FK_MENU_ITEM)->r, fake_mrect(m, 0, 0)),
		"au bon endroit");
	fake_keys(&u.r, VK_DOWN, 0, 5);
	fake_press(&u.r, VK_RIGHT, 0);
	fake_log_clear();
	ctl_invalidate(&u.r, m);
	ctl_paint(&u.r);
	h_eq_i64("deux panneaux", fake_log_kind(FK_MENU_PANEL), 2);
	fake_done(&u);
}

static void	texts(void)
{
	t_fakeui	u;

	fake_begin(&u, 300, 250);
	fake_menu(&u, 20, 20);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("7 libelles + l'ombre du desactive", fake_log_kind(FK_TEXT), 8);
	h_eq_str("mnemonique retire", fake_log_get(1)->text, "Nouveau");
	fake_keys(&u.r, VK_DOWN, 0, 1);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("element actif : encre du theme", fake_log_get(2)->a,
		(int)luna_color_text());
	fake_done(&u);
}

static void	marks(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_rect		row;
	t_rect		gut;

	fake_begin(&u, 300, 250);
	m = fake_menu(&u, 20, 20);
	ctl_paint(&u.r);
	row = fake_mrect(m, 0, 5);
	gut = rect_make(row.x, row.y, CTL_MENU_GUTTER, row.h);
	h_true(fake_px_count(&u.s, gut, 0xff000000) >= 6, "coche dessinee");
	row = fake_mrect(m, 0, 8);
	gut = rect_make(row.x, row.y, CTL_MENU_GUTTER, row.h);
	h_eq_i64("pas de coche ailleurs", fake_px_count(&u.s, gut, 0xff000000), 0);
	row = fake_mrect(m, 0, 6);
	gut = rect_make(row.x + row.w - CTL_MENU_ARROW, row.y, CTL_MENU_ARROW,
			row.h);
	h_eq_i64("fleche : 16 pixels", fake_px_count(&u.s, gut, 0xff000000), 16);
	row = fake_mrect(m, 0, 3);
	h_eq_i64("separateur", fake_px_count(&u.s, row, gfx_lerp(0xffece9d8,
				0xff000000, 120)), row.w - 2 * CTL_MENU_PAD);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_paint");
	h_run("panneaux et element actif", panels_and_hot);
	h_run("libelles, ombre du desactive, encre au survol", texts);
	h_run("coche, fleche, separateur", marks);
	return (h_end());
}
