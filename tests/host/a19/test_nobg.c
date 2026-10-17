#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

#define BACKDROP 0xff112233u
#define SURF_W 160
#define SURF_H 100

static void	paint_over(t_fakeui *u)
{
	fake_px_fill(&u->s, BACKDROP);
	ctl_invalidate(&u->r, u->r.root);
	ctl_paint(&u->r);
}

static void	default_root(void)
{
	t_fakeui	u;
	t_rect		all;

	fake_begin(&u, SURF_W, SURF_H);
	all = rect_make(0, 0, SURF_W, SURF_H);
	paint_over(&u);
	h_eq_i64("defaut : face de fenetre sur toute la surface",
		fake_px_count(&u.s, all, luna_color_window()), SURF_W * SURF_H);
	h_eq_i64("defaut : rien du contenu present n'est conserve",
		fake_px_count(&u.s, all, BACKDROP), 0);
	fake_done(&u);
}

static void	root_without_back(void)
{
	t_fakeui	u;
	t_rect		all;

	fake_begin(&u, SURF_W, SURF_H);
	all = rect_make(0, 0, SURF_W, SURF_H);
	ctl_set_flag(&u.r, u.r.root, CTL_NOBACK, true);
	paint_over(&u);
	h_eq_i64("racine sans fond : le contenu present reste",
		fake_px_count(&u.s, all, BACKDROP), SURF_W * SURF_H);
	h_eq_i64("racine sans fond : aucune face peinte",
		fake_px_count(&u.s, all, luna_color_window()), 0);
	ctl_set_flag(&u.r, u.r.root, CTL_NOBACK, false);
	paint_over(&u);
	h_eq_i64("propriete retiree : la face est repeinte",
		fake_px_count(&u.s, all, luna_color_window()), SURF_W * SURF_H);
	fake_done(&u);
}

static void	mixed_tree(void)
{
	t_fakeui	u;
	t_ctlspec	sp;
	t_ctl		*back;
	t_rect		all;

	fake_begin(&u, SURF_W, SURF_H);
	all = rect_make(0, 0, SURF_W, SURF_H);
	ctl_set_flag(&u.r, u.r.root, CTL_NOBACK, true);
	fake_add(&u, fake_spec(CT_PANEL, 3, rect_make(20, 20, 60, 40), NULL));
	sp = fake_spec(CT_PANEL, 4, rect_make(100, 50, 50, 40), NULL);
	sp.flags |= CTL_NOBACK;
	back = fake_add(&u, sp);
	fake_addp(&u, back,
		fake_spec(CT_BUTTON, 5, rect_make(110, 60, 30, 20), ""));
	paint_over(&u);
	h_eq_i64("panneau par defaut : face de fenetre",
		fake_px_count(&u.s, rect_make(20, 20, 60, 40),
			luna_color_window()), 60 * 40);
	h_eq_i64("panneau sans fond : contenu conserve hors du bouton",
		fake_px_count(&u.s, rect_make(100, 50, 50, 40), BACKDROP),
		50 * 40 - 22 * 12);
	h_eq_i64("arbre mixte : seuls la face et le bouton changent",
		fake_px_count(&u.s, all, BACKDROP),
		SURF_W * SURF_H - 60 * 40 - 22 * 12);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/nobg");
	h_run("panneau racine par defaut : fond de fenetre", default_root);
	h_run("racine sans fond : table etat de la propriete",
		root_without_back);
	h_run("arbre mixte : panneau avec et sans fond", mixed_tree);
	return (h_end());
}
