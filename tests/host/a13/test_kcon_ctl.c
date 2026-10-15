#include "kfix.h"

static void	newline(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "ab\ncd");
	h_eq_i64("colonne apres ab/cd", f.k.col, 2);
	h_eq_i64("ligne apres ab/cd", f.k.row, 1);
	h_true(kfix_is(&f, 0, 'd', 1001), "d en (1,1)");
	h_true(kfix_is(&f, 1, 'c', 1000), "c en (0,1)");
	h_true(kfix_is(&f, 2, 'b', 1), "b en (1,0)");
	kfix_close(&f);
}

static void	carriage_return(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "abc\rX");
	h_true(kfix_is(&f, 0, 'X', 0), "X ecrase la colonne 0");
	h_eq_i64("colonne apres retour chariot", f.k.col, 1);
	h_eq_i64("ligne inchangee", f.k.row, 0);
	h_true(kfix_cell_fg(&f, 2, 0) > 0, "c reste visible");
	kfix_close(&f);
}

static void	backspace(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "ab\b\bX");
	h_true(kfix_is(&f, 0, 'X', 0), "X remplace a");
	kfix_feed(&f, "\r\b\b\bQ");
	h_true(kfix_is(&f, 0, 'Q', 0), "retour arriere au debut de ligne");
	h_eq_i64("colonne apres Q", f.k.col, 1);
	kfix_feed(&f, "\n0123456789\b");
	h_eq_i64("retour arriere apres la derniere colonne", f.k.col, 9);
	kfix_close(&f);
}

static void	other_controls(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "a\x01\x1b\x7f\xc2\x85" "b");
	h_true(kfix_is(&f, 0, 'b', 1), "b juste apres a");
	h_true(kfix_is(&f, 1, 'a', 0), "a en colonne 0");
	kfix_feed_n(&f, "c\0d", 3);
	h_true(kfix_is(&f, 0, 'd', 3), "octet nul ignore");
	h_eq_i64("colonne finale", f.k.col, 4);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_ctl");
	h_run("ctl retour a la ligne", newline);
	h_run("ctl retour chariot", carriage_return);
	h_run("ctl retour arriere", backspace);
	h_run("ctl autres controles ignores", other_controls);
	return (h_end());
}
