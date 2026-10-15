#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	click_positions(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "abcdef");
	ed = e->priv;
	fake_click(&u.r, 8 + 13, 15);
	h_eq_i64("clic entre 2 et 3, plus pres de 2", ed->caret, 2);
	fake_click(&u.r, 8 + 16, 15);
	h_eq_i64("clic plus pres de 3", ed->caret, 3);
	fake_click(&u.r, 150, 15);
	h_eq_i64("clic a droite du texte", ed->caret, 6);
	fake_click(&u.r, 6, 15);
	h_eq_i64("clic a gauche du texte", ed->caret, 0);
	h_true(ed->anchor == ed->caret, "pas de selection apres un clic");
	ctl_set_text(&u.r, e, "\xc3\xa9\xe2\x82\xac\xf0\x9f\x98\x80");
	fake_click(&u.r, 8 + 10, 15);
	h_eq_i64("clic sur du multi-octets : frontiere", ed->caret, 5);
	fake_done(&u);
}

static void	drag_and_double_click(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "abcdef");
	ed = e->priv;
	fake_down(&u.r, 8 + 13, 15);
	h_true(u.r.capture == e, "capture pendant le glisse");
	fake_move(&u.r, 8 + 31, 15);
	h_true(ed->anchor == 2 && ed->caret == 5, "glisse : selection 2..5");
	fake_up(&u.r, 8 + 31, 15);
	h_true(u.r.capture == NULL, "capture rendue");
	fake_move(&u.r, 8, 15);
	h_true(ed->anchor == 2 && ed->caret == 5, "mouvement sans appui : rien");
	ctl_set_clock(&u.r, fake_clock_now);
	fake_click(&u.r, 8 + 13, 15);
	fake_clock_set(100000000);
	fake_click(&u.r, 8 + 13, 15);
	h_true(ed->anchor == 0 && ed->caret == 6, "double clic : tout");
	fake_done(&u);
}

static void	horizontal_scroll(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	ctl_set_rect(&u.r, e, rect_make(5, 5, 40, 20));
	ed = e->priv;
	fake_type(&u.r, "abcdefghij");
	h_eq_i64("defilement : caret visible a droite", ed->scroll, 60 - 34 + 1);
	fake_press(&u.r, VK_HOME, 0);
	h_eq_i64("Debut : retour a 0", ed->scroll, 0);
	fake_press(&u.r, VK_END, 0);
	fake_keys(&u.r, VK_LEFT, 0, 3);
	h_true(ed->scroll > 0, "gauche : defilement conserve");
	fake_keys(&u.r, VK_LEFT, 0, 7);
	h_eq_i64("caret au bord gauche : defilement 0", ed->scroll, 0);
	ctl_set_text(&u.r, e, "ab");
	h_eq_i64("texte court : pas de defilement", ed->scroll, 0);
	fake_done(&u);
}

static void	paint_after_scroll(void)
{
	t_fakeui	u;
	t_ctl		*e;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	ctl_set_rect(&u.r, e, rect_make(5, 5, 40, 20));
	fake_type(&u.r, "abcdefghij");
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("texte dessine d'un bloc", fake_log_last(FK_TEXT)->b, 10);
	h_eq_i64("decale du defilement", fake_log_last(FK_TEXT)->r.x, 5 + 3 - 27);
	h_true(fake_rect_eq(fake_log_last(FK_TEXT)->clip, rect_make(7, 7, 36, 16)),
		"texte ecrete a l'interieur du cadre");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit_mouse");
	h_run("clic : position du caret", click_positions);
	h_run("glisse et double clic", drag_and_double_click);
	h_run("defilement horizontal", horizontal_scroll);
	h_run("dessin avec defilement", paint_after_scroll);
	return (h_end());
}
