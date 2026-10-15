#include <string.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	copy_paste(void)
{
	t_fakeui	u;
	t_ctl		*e;
	char		clip[CTL_TEXT_MAX];

	fake_begin(&u, 200, 40);
	ctl_clip_clear();
	e = fake_edit(&u, "h\xc3\xa9llo");
	fake_press(&u.r, 'A', INPM_CTRL);
	fake_press(&u.r, 'C', INPM_CTRL);
	ctl_clip_get(clip, sizeof(clip));
	h_eq_str("copie", clip, "h\xc3\xa9llo");
	fake_press(&u.r, VK_BACK, 0);
	h_eq_str("efface", e->text, "");
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("collage", e->text, "h\xc3\xa9llo");
	fake_done(&u);
}

static void	cut_paste(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;
	char		clip[CTL_TEXT_MAX];

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "h\xc3\xa9llo");
	ed = e->priv;
	ed->anchor = 1;
	ed->caret = 3;
	fake_press(&u.r, 'X', INPM_CTRL);
	ctl_clip_get(clip, sizeof(clip));
	h_eq_str("coupe : presse-papiers", clip, "\xc3\xa9");
	h_eq_str("coupe : texte", e->text, "hllo");
	fake_press(&u.r, VK_END, 0);
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("coller en fin", e->text, "hllo\xc3\xa9");
	h_eq_i64("deux notifications", fake_cmd_find(1, CN_CHANGED), 2);
	fake_done(&u);
}

static void	paste_sanitized(void)
{
	t_fakeui	u;
	t_ctl		*e;
	t_edit		*ed;

	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "xy");
	ed = e->priv;
	ctl_clip_set("a\nb\x01\xff" "c\t\x7f" "d", 9);
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("controles et octets invalides retires", e->text, "xyabcd");
	ctl_clip_clear();
	ed->anchor = 0;
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("presse-papiers vide : rien", e->text, "xyabcd");
	h_true(ed->anchor == 0 && ed->caret == 6, "la selection reste");
	ctl_clip_set("\n\t", 2);
	fake_press(&u.r, 'V', INPM_CTRL);
	h_eq_str("rien d'imprimable : rien", e->text, "xyabcd");
	fake_done(&u);
}

static void	password_mode(void)
{
	t_fakeui	u;
	t_ctl		*e;
	char		clip[CTL_TEXT_MAX];

	fake_begin(&u, 200, 40);
	ctl_clip_set("prev", 4);
	e = fake_edit(&u, "");
	ctl_set_flag(&u.r, e, CTL_PASSWORD, true);
	fake_type(&u.r, "a\xc3\xa9\xe2\x82\xac");
	h_eq_str("le texte reel est conserve", e->text, "a\xc3\xa9\xe2\x82\xac");
	ctl_paint(&u.r);
	h_eq_str("affichage masque", fake_log_last(FK_TEXT)->text, "***");
	fake_press(&u.r, 'A', INPM_CTRL);
	fake_press(&u.r, 'C', INPM_CTRL);
	fake_press(&u.r, 'X', INPM_CTRL);
	ctl_clip_get(clip, sizeof(clip));
	h_eq_str("copier et couper refuses", clip, "prev");
	h_eq_str("couper ne retire rien", e->text, "a\xc3\xa9\xe2\x82\xac");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/edit_clip");
	h_run("copier puis coller", copy_paste);
	h_run("couper puis coller", cut_paste);
	h_run("collage nettoye", paste_sanitized);
	h_run("mot de passe : masque, copie refusee", password_mode);
	return (h_end());
}
