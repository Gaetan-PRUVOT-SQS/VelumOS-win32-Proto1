#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	cascade_no_leak(void)
{
	t_fakeui	u;
	int			base;
	int			id;

	fake_begin(&u, 200, 150);
	fake_add(&u, fake_spec(CT_LABEL, 50, rect_make(0, 140, 10, 8), "z"));
	base = fake_mem_live();
	fake_scene(&u);
	h_true(fake_mem_live() > base + 10, "la scene alloue");
	ctl_menu_close(&u.r, ctl_find(&u.r, 12));
	ctl_remove(&u.r, ctl_find(&u.r, 1));
	h_eq_i64("retour exact au niveau initial", fake_mem_live(), base);
	id = 1;
	while (id <= 12)
	{
		h_true(ctl_find(&u.r, id) == NULL, "controle retire");
		id++;
	}
	h_true(ctl_find(&u.r, 50) != NULL, "le frere reste");
	fake_done(&u);
}

static void	unlink_positions(void)
{
	t_fakeui	u;
	t_ctl		*c[3];
	int			i;

	fake_begin(&u, 100, 100);
	i = 0;
	while (i < 3)
	{
		c[i] = fake_add(&u, fake_spec(CT_LABEL, 1 + i, rect_make(0, 0, 9, 9),
					""));
		i++;
	}
	ctl_remove(&u.r, c[1]);
	h_true(c[0]->next == c[2], "milieu retire");
	ctl_remove(&u.r, c[0]);
	h_true(u.r.root->first == c[2], "premier retire");
	ctl_remove(&u.r, c[2]);
	h_true(u.r.root->first == NULL, "dernier retire");
	ctl_remove(&u.r, u.r.root);
	h_true(u.r.root != NULL, "la racine ne se retire pas");
	fake_done(&u);
}

static void	refs_cleared(void)
{
	t_fakeui	u;
	t_ctl		*p;
	t_ctl		*b;
	t_rect		pr;

	fake_begin(&u, 200, 100);
	pr = rect_make(0, 0, 200, 50);
	p = fake_add(&u, fake_spec(CT_PANEL, 1, pr, ""));
	b = fake_addp(&u, p, fake_spec(CT_BUTTON, 2, rect_make(5, 5, 50, 20), "b"));
	ctl_focus(&u.r, fake_addp(&u, p, fake_spec(CT_EDIT, 3,
				rect_make(70, 5, 100, 20), "")));
	fake_move(&u.r, 10, 10);
	h_true(u.r.hot == b, "survol");
	fake_down(&u.r, 10, 10);
	h_true(u.r.capture == b && u.r.focus == b, "capture et focus");
	ctl_paint(&u.r);
	ctl_remove(&u.r, p);
	h_true(!u.r.focus && !u.r.hot && !u.r.capture, "references a zero");
	h_true(fake_dirty_is(&u.r, &pr, 1), "zone sale = rect retire");
	h_true(!fake_up(&u.r, 10, 10), "relachement sans cible");
	fake_done(&u);
}

static void	foreign_and_menu(void)
{
	t_fakeui	u;
	t_fakeui	o;
	t_ctl		*x;
	t_ctl		*m;

	fake_begin(&u, 200, 150);
	fake_ui_open(&o, 50, 50);
	x = fake_add(&o, fake_spec(CT_BUTTON, 1, rect_make(0, 0, 9, 9), "x"));
	ctl_remove(&u.r, x);
	h_true(ctl_find(&o.r, 1) == x, "retrait d'un controle etranger ignore");
	fake_ui_close(&o);
	m = fake_scene_menu(&u);
	h_true(m && u.r.capture == m && u.r.focus == m, "menu ouvert");
	ctl_remove(&u.r, m);
	h_true(!u.r.capture && !u.r.focus, "menu retire a vif");
	fake_scene(&u);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/remove");
	h_run("retrait en cascade sans fuite", cascade_no_leak);
	h_run("detachement premier, milieu, dernier", unlink_positions);
	h_run("references focus, survol, capture remises a zero", refs_cleared);
	h_run("retrait etranger et menu ouvert", foreign_and_menu);
	return (h_end());
}
