#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	z_order(void)
{
	t_fakeui	u;
	t_ctl		*a;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	a = fake_btn(&u);
	b = fake_add(&u, fake_spec(CT_BUTTON, 2, rect_make(10, 10, 60, 20), "B"));
	b->cb = fake_cb;
	h_true(ctl_hit(&u.r, (t_point){15, 15}) == b, "le dernier est au dessus");
	fake_click(&u.r, 15, 15);
	h_eq_i64("B clique", fake_cmd_find(2, CN_CLICKED), 1);
	ctl_set_flag(&u.r, b, CTL_VISIBLE, false);
	h_true(ctl_hit(&u.r, (t_point){15, 15}) == a, "B masque : A");
	fake_click(&u.r, 15, 15);
	h_eq_i64("A clique", fake_cmd_find(1, CN_CLICKED), 1);
	h_true(ctl_hit(&u.r, (t_point){150, 90}) == u.r.root, "fond : la racine");
	h_true(ctl_hit(&u.r, (t_point){-1, 5}) == NULL, "hors surface");
	h_true(ctl_hit(&u.r, (t_point){200, 5}) == NULL, "limite exclue");
	fake_done(&u);
}

static void	nested_clipping(void)
{
	t_fakeui	u;
	t_ctl		*p;
	t_ctl		*c;

	fake_begin(&u, 200, 100);
	p = fk_c(&u, CT_PANEL, 5, rect_make(50, 0, 60, 100));
	c = fake_addp(&u, p, fake_spec(CT_BUTTON, 6, rect_make(60, 10, 100, 20),
				""));
	h_true(ctl_hit(&u.r, (t_point){70, 15}) == c, "enfant dans le parent");
	h_true(ctl_hit(&u.r, (t_point){130, 15}) == u.r.root, "enfant ecrete");
	h_true(!fake_click(&u.r, 55, 60), "clic sur un panneau : non consomme");
	h_true(fake_click(&u.r, 70, 15), "clic sur l'enfant : consomme");
	fake_done(&u);
}

static void	disabled_swallows(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = fake_btn(&u);
	ctl_set_flag(&u.r, b, CTL_ENABLED, false);
	h_true(fake_move(&u.r, 15, 15), "survol d'un desactive : consomme");
	h_true(u.r.hot == NULL, "pas de survol");
	h_true(fake_down(&u.r, 15, 15), "appui : avale");
	h_true(fake_up(&u.r, 15, 15), "relachement : avale");
	h_eq_i64("aucune notification", fake_cmd_count(), 0);
	fake_done(&u);
}

static void	hot_only_for_buttons(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_ctl		*rd;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	rd = fk_c(&u, CT_RADIO, 9, rect_make(120, 10, 60, 14));
	fake_move(&u.r, 15, 15);
	h_true(u.r.hot == c[0], "bouton survolable");
	fake_move(&u.r, 15, 45);
	h_true(u.r.hot == NULL, "etiquette : pas de survol");
	fake_move(&u.r, 15, 65);
	h_true(u.r.hot == NULL, "edit : pas de survol");
	fake_move(&u.r, 15, 95);
	h_true(u.r.hot == c[3], "case survolable");
	fake_move(&u.r, 125, 15);
	h_true(u.r.hot == rd, "radio survolable");
	h_true((rd->state & CTL_ST_HOT) && !(c[3]->state & CTL_ST_HOT), "etats");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/hit");
	h_run("ordre Z et masquage", z_order);
	h_run("enfant ecrete par son parent", nested_clipping);
	h_run("controle desactive : evenements avales", disabled_swallows);
	h_run("survol reserve aux boutons, cases, radios", hot_only_for_buttons);
	return (h_end());
}
