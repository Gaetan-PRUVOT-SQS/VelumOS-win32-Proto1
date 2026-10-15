#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	alignment_and_bold(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 60);
	l = fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(10, 10, 100, 14), "ab"));
	ctl_paint(&u.r);
	h_eq_i64("gauche : x", fake_log_last(FK_TEXT)->r.x, 10);
	h_eq_i64("centre vertical : y", fake_log_last(FK_TEXT)->r.y, 11);
	ctl_set_flag(&u.r, l, CTL_ALIGN_CENTER, true);
	ctl_paint(&u.r);
	h_eq_i64("centre : x", fake_log_last(FK_TEXT)->r.x, 10 + (100 - 12) / 2);
	ctl_set_flag(&u.r, l, CTL_ALIGN_CENTER, false);
	ctl_set_flag(&u.r, l, CTL_ALIGN_RIGHT, true);
	ctl_paint(&u.r);
	h_eq_i64("droite : x", fake_log_last(FK_TEXT)->r.x, 10 + 100 - 12);
	ctl_set_flag(&u.r, l, CTL_BOLD, true);
	ctl_paint(&u.r);
	h_eq_i64("gras : largeur 7 par caractere", fake_log_last(FK_TEXT)->r.w, 14);
	fake_done(&u);
}

static void	fit_and_ellipsis(void)
{
	t_fakeui	u;
	t_ctl		*l;
	const char	*long_text;

	long_text = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	fake_begin(&u, 200, 60);
	l = fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(10, 10, 60, 14),
				long_text));
	ctl_paint(&u.r);
	h_eq_i64("sans ellipse : 10 caracteres", fake_log_last(FK_TEXT)->b, 10);
	ctl_set_flag(&u.r, l, CTL_ELLIPSIS, true);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("ellipse : deux dessins", fake_log_count(), 2);
	h_eq_i64("ellipse : 9 caracteres", fake_log_get(0)->b, 9);
	h_eq_i64("ellipse : x des points", fake_log_get(1)->r.x, 10 + 54);
	h_eq_str("ellipse : U+2026", fake_log_get(1)->text, "\xe2\x80\xa6");
	ctl_set_text(&u.r, l, "\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9");
	ctl_set_rect(&u.r, l, rect_make(10, 10, 13, 14));
	ctl_set_flag(&u.r, l, CTL_ELLIPSIS, false);
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("2 caracteres de 2 octets", fake_log_get(0)->b, 4);
	fake_done(&u);
}

static void	disabled_engraved(void)
{
	t_fakeui	u;
	t_ctl		*l;

	fake_begin(&u, 200, 60);
	l = fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(10, 10, 60, 14), "off"));
	ctl_paint(&u.r);
	fake_log_clear();
	ctl_set_flag(&u.r, l, CTL_ENABLED, false);
	ctl_paint(&u.r);
	h_eq_i64("estampe : deux dessins", fake_log_kind(FK_TEXT), 2);
	h_eq_i64("ombre blanche", fake_log_get(0)->a, (int)0xffffffff);
	h_eq_i64("ombre decalee en x", fake_log_get(0)->r.x, 11);
	h_eq_i64("ombre decalee en y", fake_log_get(0)->r.y, 12);
	h_eq_i64("texte gris", (uint32_t)fake_log_get(1)->a,
		gfx_lerp(0xffece9d8, 0xff000000, 120));
	h_eq_i64("texte a sa place", fake_log_get(1)->r.x, 10);
	fake_done(&u);
}

static void	mnemonic_underline(void)
{
	t_fakeui	u;
	t_ctl		*l;
	t_rect		row;

	fake_begin(&u, 200, 60);
	l = fake_add(&u, fake_spec(CT_LABEL, 1, rect_make(10, 10, 100, 14),
				"&Fichier"));
	ctl_paint(&u.r);
	h_eq_str("texte sans &", fake_log_last(FK_TEXT)->text, "Fichier");
	row = rect_make(10, 11 + 10, 6, 1);
	h_eq_i64("souligne 1 caractere", fake_px_count(&u.s, row, 0xff000000), 6);
	row = rect_make(16, 11 + 10, 40, 1);
	h_eq_i64("pas plus", fake_px_count(&u.s, row, 0xff000000), 0);
	ctl_set_text(&u.r, l, "A&&B");
	ctl_paint(&u.r);
	h_eq_str("&& litteral", fake_log_last(FK_TEXT)->text, "A&B");
	row = rect_make(10, 21, 40, 1);
	h_eq_i64("aucun souligne", fake_px_count(&u.s, row, 0xff000000), 0);
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/label");
	h_run("alignements et gras", alignment_and_bold);
	h_run("troncature, ellipse, utf-8", fit_and_ellipsis);
	h_run("texte desactive estampe", disabled_engraved);
	h_run("mnemonique souligne", mnemonic_underline);
	return (h_end());
}
