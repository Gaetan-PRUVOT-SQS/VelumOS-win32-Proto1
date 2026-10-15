#include "kfix.h"

static void	exact_last_column(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "xxxxxxxxxx");
	h_eq_i64("colonne apres 10 caracteres", f.k.col, 10);
	h_eq_i64("ligne apres 10 caracteres", f.k.row, 0);
	kfix_feed(&f, "\n");
	h_eq_i64("pas de ligne vide apres retour", f.k.row, 1);
	h_eq_i64("colonne apres retour", f.k.col, 0);
	h_eq_u64("aucun glyphe sur la ligne 1", kfix_cell_fg(&f, 0, 1), 0);
	kfix_close(&f);
}

static void	one_past_last_column(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "xxxxxxxxxxy");
	h_eq_i64("ligne apres 11 caracteres", f.k.row, 1);
	h_eq_i64("colonne apres 11 caracteres", f.k.col, 1);
	h_true(kfix_is(&f, 0, 'y', 1000), "11e caractere en (0,1)");
	kfix_close(&f);
}

static void	one_before_last_column(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "xxxxxxxxxyz");
	h_true(kfix_is(&f, 1, 'y', 9), "y en derniere colonne");
	h_true(kfix_is(&f, 0, 'z', 1000), "z passe a la ligne");
	kfix_close(&f);
}

static void	empty_and_null(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed_n(&f, "", 0);
	kfix_feed_n(&f, "abc", 0);
	h_eq_u64("aucun glyphe", g_ffont.glyphs, 0);
	h_eq_i64("colonne inchangee", f.k.col, 0);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_wrap");
	h_run("wrap derniere colonne exacte", exact_last_column);
	h_run("wrap colonne + 1", one_past_last_column);
	h_run("wrap colonne - 1", one_before_last_column);
	h_run("wrap ecriture vide", empty_and_null);
	return (h_end());
}
