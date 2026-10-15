#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	invalid_types_and_rects(void)
{
	t_fakeui	u;
	t_rect		ok;
	int32_t		m;

	fake_begin(&u, 100, 100);
	ok = rect_make(0, 0, 10, 10);
	m = CTL_COORD_MAX;
	h_true(fk_no(&u, fake_spec(CT_TYPES, 1, ok, "x")), "type = CT_TYPES");
	h_true(fk_no(&u, fake_spec(0xffffffffu, 1, ok, "x")), "type enorme");
	h_true(fk_no(&u, fake_spec(CT_IMAGE, 1, ok, "x")), "CT_IMAGE non gere");
	h_true(fk_no(&u, fake_spec(CT_MENU, 1, ok, "x")), "CT_MENU : popup seul");
	h_true(fk_no(&u, fake_spec(CT_LABEL, 1, rect_make(0, 0, -1, 5), "")),
		"largeur negative");
	h_true(fk_no(&u, fake_spec(CT_LABEL, 1, rect_make(0, 0, 5, -1), "")),
		"hauteur negative");
	h_true(fk_no(&u, fake_spec(CT_LABEL, 1, rect_make(m + 1, 0, 5, 5), "")),
		"x > max");
	h_true(fk_no(&u, fake_spec(CT_LABEL, 1, rect_make(0, -m - 1, 5, 5), "")),
		"y < -max");
	h_true(fk_no(&u, fake_spec(CT_LABEL, 1, rect_make(0, 0, m + 1, 5), "")),
		"largeur > max");
	fake_done(&u);
}

static void	boundary_rects(void)
{
	t_fakeui	u;
	int32_t		m;

	fake_begin(&u, 100, 100);
	m = CTL_COORD_MAX;
	h_true(fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(m, m, m, m), ""))
		!= NULL, "valeurs limites hautes acceptees");
	h_true(fake_add(&u, fake_spec(CT_LABEL, 2, rect_make(-m, -m, 0, 0), ""))
		!= NULL, "limite basse et taille nulle acceptees");
	h_true(ctl_paint(&u.r).w > 0, "peinture avec rectangles extremes");
	h_true(fake_dirty_is(&u.r, NULL, 0), "dirty vide apres peinture");
	fake_done(&u);
}

static void	ids_and_parents(void)
{
	t_fakeui	u;
	t_fakeui	other;
	t_ctl		*b;
	t_ctl		*foreign;

	fake_begin(&u, 100, 100);
	b = fake_add(&u, fake_spec(CT_BUTTON, 5, rect_make(0, 0, 10, 10), "b"));
	h_true(fk_no(&u, fake_spec(CT_BUTTON, 5, rect_make(0, 0, 9, 9), "c")),
		"id deja pris");
	h_true(fake_addp(&u, b, fake_spec(CT_LABEL, 6, rect_make(0, 0, 5, 5), ""))
		== NULL, "un bouton n'est pas un conteneur");
	ctl_remove(&u.r, b);
	h_true(fake_add(&u, fake_spec(CT_BUTTON, 5, rect_make(0, 0, 9, 9), "c"))
		!= NULL, "id reutilisable apres retrait");
	fake_ui_open(&other, 50, 50);
	foreign = fake_add(&other, fake_spec(CT_PANEL, 1, rect_make(0, 0, 9, 9),
				""));
	h_true(fake_addp(&u, foreign, fake_spec(CT_LABEL, 9, rect_make(0, 0, 5, 5),
				"")) == NULL, "parent d'un autre arbre refuse");
	fake_ui_close(&other);
	fake_done(&u);
}

static void	depth_limit(void)
{
	t_fakeui	u;
	t_ctl		*cur;
	t_ctl		*next;
	t_ctlspec	sp;
	int			n;

	fake_begin(&u, 100, 100);
	sp = fake_spec(CT_PANEL, 0, rect_make(0, 0, 9, 9), "");
	cur = u.r.root;
	n = 0;
	next = fake_addp(&u, cur, sp);
	while (next)
	{
		n++;
		cur = next;
		next = fake_addp(&u, cur, sp);
	}
	h_eq_i64("profondeur maximale", n, CTL_DEPTH_MAX - 1);
	h_true(ctl_paint(&u.r).w > 0, "peinture d'un arbre profond");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/tree_invalid");
	h_run("specs invalides : type, rectangle", invalid_types_and_rects);
	h_run("rectangles aux valeurs limites", boundary_rects);
	h_run("ids uniques, parents invalides", ids_and_parents);
	h_run("profondeur maximale de l'arbre", depth_limit);
	return (h_end());
}
