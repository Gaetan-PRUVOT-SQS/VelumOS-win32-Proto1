#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	no_change_no_dirty(void)
{
	t_fakeui	u;
	t_ctl		*c[4];

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	h_true(fake_dirty_is(&u.r, NULL, 0), "propre apres la peinture");
	ctl_set_text(&u.r, c[1], "L");
	ctl_set_flag(&u.r, c[0], CTL_VISIBLE, true);
	ctl_set_flag(&u.r, c[0], CTL_FOCUSABLE, false);
	ctl_set_flag(&u.r, c[0], CTL_FOCUSABLE, true);
	h_eq_i64("set_rect identique", ctl_set_rect(&u.r, c[1], c[1]->rect), 0);
	ctl_focus(&u.r, NULL);
	ctl_progress_set(&u.r, c[0], 10);
	h_true(fake_dirty_is(&u.r, NULL, 0), "aucune modification, aucune zone");
	fake_done(&u);
}

static void	property_changes(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		want[2];

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	ctl_set_text(&u.r, c[1], "X");
	want[0] = c[1]->rect;
	h_true(fake_dirty_is(&u.r, want, 1), "texte : rect du label");
	ctl_paint(&u.r);
	ctl_set_flag(&u.r, c[0], CTL_DEFAULT, true);
	want[0] = c[0]->rect;
	h_true(fake_dirty_is(&u.r, want, 1), "drapeau visuel : rect du bouton");
	ctl_paint(&u.r);
	want[1] = rect_make(10, 42, 80, 12);
	want[0] = c[1]->rect;
	ctl_set_rect(&u.r, c[1], want[1]);
	h_true(fake_dirty_is(&u.r, want, 2), "deplacement : ancien et nouveau");
	ctl_paint(&u.r);
	ctl_set_flag(&u.r, c[0], CTL_VISIBLE, false);
	want[0] = c[0]->rect;
	h_true(fake_dirty_is(&u.r, want, 1), "masquage : rect du bouton");
	fake_done(&u);
}

static void	pointer_and_focus(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		want[2];

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	fake_move(&u.r, 15, 15);
	want[0] = c[0]->rect;
	h_true(fake_dirty_is(&u.r, want, 1), "entree sur le bouton");
	ctl_paint(&u.r);
	fake_move(&u.r, 15, 65);
	h_true(fake_dirty_is(&u.r, want, 1), "sortie du bouton");
	ctl_paint(&u.r);
	fake_move(&u.r, 150, 110);
	h_true(fake_dirty_is(&u.r, NULL, 0), "zone vide : rien");
	ctl_focus(&u.r, c[0]);
	h_true(fake_dirty_is(&u.r, want, 1), "focus : rect du bouton");
	ctl_paint(&u.r);
	want[1] = c[2]->rect;
	ctl_focus(&u.r, c[2]);
	h_true(fake_dirty_is(&u.r, want, 2), "focus deplace : ancien et nouveau");
	fake_done(&u);
}

static void	typing_and_clicks(void)
{
	t_fakeui	u;
	t_ctl		*c[4];
	t_rect		want;

	fake_begin(&u, 200, 120);
	fake_dscene(&u, c);
	ctl_focus(&u.r, c[2]);
	ctl_paint(&u.r);
	fake_type(&u.r, "ab");
	want = c[2]->rect;
	h_true(fake_dirty_is(&u.r, &want, 1), "frappe : rect de l'edit");
	ctl_paint(&u.r);
	fake_press(&u.r, VK_HOME, 0);
	h_true(fake_dirty_is(&u.r, &want, 1), "Debut : caret deplace");
	ctl_paint(&u.r);
	h_true(fake_press(&u.r, VK_HOME, 0), "Debut est consomme");
	h_true(fake_dirty_is(&u.r, NULL, 0), "Debut sans effet : rien");
	fake_press(&u.r, VK_TAB, 0);
	h_true(fake_dirty_covers(&u.r, c[2]->rect), "Tab : ancien focus");
	h_true(fake_dirty_covers(&u.r, c[3]->rect), "Tab : nouveau focus");
	h_true(!fake_dirty_covers(&u.r, c[0]->rect), "Tab : bouton propre");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/dirty");
	h_run("aucune modification : zone vide", no_change_no_dirty);
	h_run("texte, drapeaux, deplacement : zones exactes", property_changes);
	h_run("survol et focus : zones exactes", pointer_and_focus);
	h_run("frappe, touches, Tab : zones exactes", typing_and_clicks);
	return (h_end());
}
