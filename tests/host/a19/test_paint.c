#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	paint_only_dirty(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		r;
	t_rect		all;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	fake_px_fill(&u.s, SENTINEL);
	ctl_set_text(&u.r, c[1], "Z");
	r = ctl_paint(&u.r);
	all = rect_make(0, 0, 200, 120);
	h_true(fake_rect_eq(r, c[1]->rect), "enveloppe renvoyee = zone sale");
	h_eq_i64("hors zone : intact", fake_px_count(&u.s, all, SENTINEL),
		200 * 120 - 80 * 12);
	h_eq_i64("dans la zone : tout peint", fake_px_count(&u.s, r, SENTINEL), 0);
	h_eq_i64("bouton non redessine", fake_log_kind(FK_BUTTON), 0);
	h_eq_i64("case non redessinee", fake_log_kind(FK_CHECK), 0);
	h_eq_i64("edit non redessine", fake_log_kind(FK_EDIT_FRAME), 0);
	h_eq_i64("peinture suivante vide", ctl_paint(&u.r).w, 0);
	h_eq_i64("rien de plus dessine", fake_log_kind(FK_TEXT), 1);
	fake_done(&u);
}

static void	clips_and_restore(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		saved;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	saved = rect_make(5, 5, 100, 100);
	gfx_set_clip(&u.s, saved);
	ctl_invalidate(&u.r, c[0]);
	ctl_invalidate(&u.r, c[3]);
	ctl_paint(&u.r);
	h_true(fake_rect_eq(u.s.clip, saved), "clip de la surface restaure");
	h_true(fake_rect_eq(fake_log_last(FK_BUTTON)->clip, c[0]->rect),
		"bouton ecrete a sa zone");
	h_true(fake_rect_eq(fake_log_last(FK_CHECK)->clip, c[3]->rect),
		"case ecretee a sa zone");
	h_eq_i64("une seule fois chacun", fake_log_kind(FK_BUTTON), 1);
	fake_done(&u);
}

static void	nesting_and_hidden(void)
{
	t_fakeui	u;
	t_ctl		*p;
	t_rect		part;

	fake_begin(&u, 200, 120);
	part = rect_make(80, 20, 30, 20);
	p = fake_add(&u, fake_spec(CT_PANEL, 1, rect_make(10, 10, 100, 60), ""));
	fake_addp(&u, p, fake_spec(CT_BUTTON, 2, rect_make(20, 20, 50, 20), "b"));
	fake_addp(&u, p, fake_spec(CT_LABEL, 3, rect_make(80, 20, 60, 20), "abc"));
	ctl_paint(&u.r);
	h_true(fake_rect_eq(fake_log_last(FK_TEXT)->clip, part),
		"enfant ecrete par son parent");
	ctl_set_flag(&u.r, ctl_find(&u.r, 3), CTL_VISIBLE, false);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("enfant masque : non dessine", fake_log_kind(FK_TEXT), 0);
	ctl_set_flag(&u.r, p, CTL_VISIBLE, false);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("parent masque : enfants caches", fake_log_kind(FK_BUTTON), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/paint");
	h_run("peinture limitee a la zone sale", paint_only_dirty);
	h_run("clips par controle, clip de surface restaure", clips_and_restore);
	h_run("parent, enfant ecrete, masquage", nesting_and_hidden);
	return (h_end());
}
