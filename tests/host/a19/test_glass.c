#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static t_ctl	*add(t_fakeui *u, uint32_t type, t_rect r, const char *text)
{
	return (fake_add(u, fake_spec(type, 7, r, text)));
}

static const t_fakecall	*paint_last(t_fakeui *u, t_ctl *c, int kind)
{
	ctl_invalidate(&u->r, c);
	fake_log_clear();
	ctl_paint(&u->r);
	return (fake_log_last(kind));
}

static void	button_ink_table(void)
{
	t_fakeui	u;
	t_ctl		*b;

	fake_begin(&u, 200, 100);
	b = add(&u, CT_BUTTON, rect_make(20, 20, 80, 32), "OK");
	ctl_set_flag(&u.r, b, CTL_ENABLED, true);
	ctl_set_flag(&u.r, b, CTL_DEFAULT, false);
	h_eq_i64("normal : encre", paint_last(&u, b, FK_TEXT)->a, (int)0xff000000);
	h_eq_i64("normal : chasse regulier", paint_last(&u, b, FK_TEXT)->r.w,
		2 * FAKE_CHAR_W);
	ctl_set_flag(&u.r, b, CTL_DEFAULT, true);
	h_eq_i64("principal : blanc", paint_last(&u, b, FK_TEXT)->a,
		(int)0xffffffff);
	h_eq_i64("principal : gras", paint_last(&u, b, FK_TEXT)->r.w,
		2 * FAKE_BOLD_W);
	ctl_set_flag(&u.r, b, CTL_ENABLED, false);
	h_eq_i64("principal desactive : encre attenuee",
		paint_last(&u, b, FK_TEXT)->a, (int)0xff808080);
	h_eq_i64("principal desactive : regulier", paint_last(&u, b, FK_TEXT)->r.w,
		2 * FAKE_CHAR_W);
	ctl_set_flag(&u.r, b, CTL_DEFAULT, false);
	h_eq_i64("desactive : encre attenuee", paint_last(&u, b, FK_TEXT)->a,
		(int)0xff808080);
	h_eq_i64("desactive : aucune ombre", fake_log_kind(FK_TEXT), 1);
	fake_done(&u);
}

static void	arrows_use_button_ink(void)
{
	t_fakeui	u;
	t_ctl		*s;
	t_rect		dec;

	fake_begin(&u, 100, 150);
	s = add(&u, CT_SCROLL, rect_make(10, 10, 17, 100), NULL);
	ctl_scroll_set(&u.r, s, 100, 10);
	paint_last(&u, s, FK_BUTTON);
	dec = s->rect;
	if (fake_log_get(0)->kind == FK_BUTTON)
		dec = fake_log_get(0)->r;
	h_eq_i64("fleche vivante : encre, 1+3+5+7 pixels",
		fake_px_count(&u.s, dec, 0xff000000), 16);
	h_eq_i64("pas d'encre attenuee", fake_px_count(&u.s, dec, 0xff808080), 0);
	ctl_scroll_set(&u.r, s, 0, 10);
	paint_last(&u, s, FK_BUTTON);
	h_eq_i64("fleche inactive : encre attenuee",
		fake_px_count(&u.s, dec, 0xff808080), 16);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/glass");
	h_run("encre du bouton : table de decision etat x principal",
		button_ink_table);
	h_run("fleches d'ascenseur : encre du bouton", arrows_use_button_ink);
	return (h_end());
}
