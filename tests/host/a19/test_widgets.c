#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	progress_values(void)
{
	t_fakeui	u;
	t_ctl		*p;
	t_rect		want;

	fake_begin(&u, 200, 100);
	p = fk_c(&u, CT_PROGRESS, 1, rect_make(10, 10, 100, 14));
	ctl_paint(&u.r);
	ctl_progress_set(&u.r, p, 40);
	want = p->rect;
	h_true(fake_dirty_is(&u.r, &want, 1), "changement : rect de la barre");
	ctl_paint(&u.r);
	h_eq_i64("pourcentage transmis", fake_log_last(FK_PROGRESS)->a, 40);
	ctl_progress_set(&u.r, p, 40);
	h_true(fake_dirty_is(&u.r, NULL, 0), "meme valeur : rien");
	ctl_progress_set(&u.r, p, 101);
	ctl_paint(&u.r);
	h_eq_i64("plafonne a 100", fake_log_last(FK_PROGRESS)->a, 100);
	ctl_progress_set(&u.r, p, 0xffffffffu);
	h_true(fake_dirty_is(&u.r, NULL, 0), "enorme : deja a 100");
	ctl_progress_set(&u.r, p, 0);
	ctl_paint(&u.r);
	h_eq_i64("zero", fake_log_last(FK_PROGRESS)->a, 0);
	fake_done(&u);
}

static void	progress_inert(void)
{
	t_fakeui	u;
	t_ctl		*p;

	fake_begin(&u, 200, 100);
	p = fk_c(&u, CT_PROGRESS, 1, rect_make(10, 10, 100, 14));
	h_true(!fake_click(&u.r, 20, 15), "clic non consomme");
	h_true(!(p->flags & CTL_FOCUSABLE), "pas focusable");
	ctl_focus(&u.r, p);
	h_true(u.r.focus == NULL, "focus refuse");
	h_true(!fake_press(&u.r, VK_TAB, 0), "Tab ne la visite pas");
	fake_done(&u);
}

static void	group_box(void)
{
	t_fakeui	u;
	t_ctl		*g;

	fake_begin(&u, 200, 100);
	g = fake_add(&u, fake_spec(CT_GROUP, 1, rect_make(10, 10, 100, 60), "Opt"));
	ctl_paint(&u.r);
	h_eq_i64("largeur de l'etiquette + 8", fake_log_last(FK_GROUP)->a,
		3 * 6 + 2 * 4);
	h_eq_i64("etiquette decalee de 8", fake_log_last(FK_TEXT)->r.x, 10 + 8);
	h_eq_i64("etiquette en haut du cadre", fake_log_last(FK_TEXT)->r.y, 10);
	ctl_set_text(&u.r, g, "");
	fake_log_clear();
	ctl_paint(&u.r);
	h_eq_i64("sans texte : pas de trou", fake_log_last(FK_GROUP)->a, 0);
	h_true(!fake_click(&u.r, 20, 30), "le cadre n'absorbe pas les clics");
	fake_done(&u);
}

static void	group_mnemonic(void)
{
	t_fakeui	u;
	t_ctl		*g;
	t_ctl		*r;

	fake_begin(&u, 200, 100);
	g = fake_add(&u, fake_spec(CT_GROUP, 1, rect_make(10, 10, 100, 60),
				"&Choix"));
	r = fake_addp(&u, g, fake_spec(CT_RADIO, 2, rect_make(15, 25, 60, 14),
				"a"));
	fake_press(&u.r, 'C', INPM_ALT);
	h_true(u.r.focus == r, "Alt+C : premier controle du groupe");
	fake_done(&u);
}

int	main(void)
{
	h_begin("a19/widgets");
	h_run("progression : valeurs et zones", progress_values);
	h_run("progression : inerte", progress_inert);
	h_run("groupe : cadre et etiquette", group_box);
	h_run("groupe : mnemonique", group_mnemonic);
	return (h_end());
}
