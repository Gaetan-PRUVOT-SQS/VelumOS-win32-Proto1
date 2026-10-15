#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	hover(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fk_mv(&u, m, 0, 0);
	h_eq_i64("survol : Nouveau", fake_msel(m, 0), 0);
	fk_mv(&u, m, 0, 3);
	h_eq_i64("separateur : inchange", fake_msel(m, 0), 0);
	fk_mv(&u, m, 0, 6);
	h_true(fake_msel(m, 0) == 6 && fake_mdepth(m) == 2, "Plus ouvre");
	fk_mv(&u, m, 1, 1);
	h_eq_i64("dans le sous-menu", fake_msel(m, 1), 1);
	h_eq_i64("le parent garde Plus", fake_msel(m, 0), 6);
	fk_mv(&u, m, 0, 0);
	h_true(fake_mdepth(m) == 1 && fake_msel(m, 0) == 0, "retour : ferme");
	fake_move(&u.r, 190, 140);
	h_eq_i64("hors du menu : inchange", fake_msel(m, 0), 0);
	fake_done(&u);
}

static void	click_pick_and_swallow(void)
{
	t_fakeui	u;
	t_ctl		*m;
	t_ctl		*b;
	t_point		p;

	fake_begin(&u, 200, 150);
	b = fake_add(&u, fake_spec(CT_BUTTON, 9, rect_make(0, 0, 200, 150), "b"));
	b->cb = fake_cb;
	m = fake_menu(&u, 20, 20);
	p = fake_mpt(m, 0, 2);
	h_true(fake_down(&u.r, p.x, p.y), "appui consomme");
	h_eq_i64("Ouvrir choisi", g_fake_picked, 2);
	h_true(ctl_find(&u.r, 100) == NULL, "menu ferme");
	fake_up(&u.r, p.x, p.y);
	h_eq_i64("le bouton dessous n'est pas clique", fake_cmd_find(9, CN_CLICKED),
		0);
	fake_done(&u);
}

static void	click_inert_and_nested(void)
{
	t_fakeui	u;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	m = fake_menu(&u, 20, 20);
	fk_dn(&u, m, 0, 4);
	fk_dn(&u, m, 0, 3);
	h_true(ctl_find(&u.r, 100) == m && fake_cmd_count() == 0, "inertes");
	fk_dn(&u, m, 0, 6);
	h_true(fake_mdepth(m) == 2 && fake_msel(m, 1) == 0, "clic sur un parent");
	fk_dn(&u, m, 1, 3);
	h_eq_i64("troisieme niveau", fake_mdepth(m), 3);
	fk_dn(&u, m, 2, 0);
	h_eq_i64("Delta choisi", g_fake_picked, 531);
	h_true(ctl_find(&u.r, 100) == NULL, "ferme");
	fake_done(&u);
}

static void	click_outside(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 150);
	b = fake_add(&u, fake_spec(CT_BUTTON, 9, rect_make(120, 80, 60, 30), "b"));
	b->cb = fake_cb;
	ctl_focus(&u.r, b);
	fake_menu(&u, 20, 20);
	h_true(fake_down(&u.r, 150, 100), "clic exterieur consomme");
	h_eq_i64("CN_CLOSED", fake_cmd_find(100, CN_CLOSED), 1);
	h_true(ctl_find(&u.r, 100) == NULL && u.r.capture == NULL, "menu ferme");
	h_true(u.r.focus == b, "focus rendu");
	fake_up(&u.r, 150, 100);
	h_eq_i64("le bouton sous le clic ne reagit pas", fake_cmd_find(9,
			CN_CLICKED), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/menu_mouse");
	h_run("survol : selection et ouverture", hover);
	h_run("clic : choix et clic avale", click_pick_and_swallow);
	h_run("clic : inertes, parents, niveaux", click_inert_and_nested);
	h_run("clic exterieur : fermeture", click_outside);
	return (h_end());
}
