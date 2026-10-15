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

static void	font_per_control(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 300, 100);
	l = add(&u, CT_LABEL, rect_make(10, 10, 250, 40), "Bonjour");
	h_eq_i64("police par defaut : 11", paint_last(&u, l, FK_TEXT)->r.h, 11);
	ctl_set_font(&u.r, l, FONT_INS_BOLD_24);
	h_true(!region_empty(&u.r.dirty), "changement : zone sale");
	h_eq_i64("titre 24/700", paint_last(&u, l, FK_TEXT)->r.h, 24);
	ctl_set_font(&u.r, l, FONT_INS_BOLD_24);
	h_true(region_empty(&u.r.dirty), "meme police : rien a redessiner");
	ctl_set_font(&u.r, l, (t_fontid)(FONT_IDS + 3));
	h_eq_i64("identifiant invalide ignore", paint_last(&u, l, FK_TEXT)->r.h,
		24);
	ctl_set_font(&u.r, l, CTL_FONT_AUTO);
	h_eq_i64("retour a la police par defaut", paint_last(&u, l, FK_TEXT)->r.h,
		11);
	ctl_set_font(NULL, l, FONT_UI);
	ctl_set_font(&u.r, NULL, FONT_UI);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/glass-font");
	h_run("police par controle : propriete, reset, invalide",
		font_per_control);
	return (h_end());
}
