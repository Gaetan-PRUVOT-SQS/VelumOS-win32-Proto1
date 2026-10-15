#include "kfix.h"

static void	decoder_without_progress(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	g_ffont.utf8_bug = 1;
	kfix_feed(&f, "abc");
	h_eq_i64("sans progres : une cellule par octet", f.k.col, 3);
	h_true(kfix_ok(&f), "curseur dans les bornes");
	kfix_close(&f);
}

static void	decoder_overshoot(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	g_ffont.utf8_bug = 2;
	kfix_feed(&f, "abc");
	h_eq_i64("depassement ramene a la fin : une cellule", f.k.col, 1);
	h_true(kfix_ok(&f), "curseur dans les bornes");
	kfix_close(&f);
}

static void	wrap_with_broken_decoder(void)
{
	t_bsod_wrap	w;

	fake_font_reset();
	g_ffont.utf8_bug = 1;
	w.text = "abc def ghi";
	w.cols = 4;
	w.max = 12;
	bsod_wrap(&w);
	h_true(w.count > 0 && w.count <= w.max, "sans progres : fin garantie");
	h_eq_i64("proprietes du decoupage", fake_wrapck(&w, w.text, 4), 0);
	g_ffont.utf8_bug = 2;
	bsod_wrap(&w);
	h_eq_i64("depassement : une seule ligne", w.count, 1);
	h_eq_i64("la ligne ne depasse pas le texte", w.lines[0].len, 11);
}

int	main(void)
{
	h_begin("a13/kcon_hostile");
	h_run("decodeur sans progres", decoder_without_progress);
	h_run("decodeur qui depasse la fin", decoder_overshoot);
	h_run("decoupage avec decodeur defaillant", wrap_with_broken_decoder);
	return (h_end());
}
