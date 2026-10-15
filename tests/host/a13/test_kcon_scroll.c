#include "kfix.h"

static void	one_scroll(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kcon_clear_all(&f.k);
	kfix_feed(&f, "A\nB\nC\nD");
	h_eq_i64("ligne avant defilement", f.k.row, 3);
	f.g.px[21 * 80 + 3] = 0xff123456;
	kfix_feed(&f, "\n");
	h_eq_u64("le contenu monte d'une ligne", f.g.px[5 * 80 + 3], 0xff123456);
	h_eq_i64("ligne apres defilement", f.k.row, 3);
	h_eq_i64("colonne apres defilement", f.k.col, 0);
	h_true(kfix_cell_fg(&f, 0, 0) > 0, "B est monte en ligne 0");
	h_true(kfix_cell_fg(&f, 0, 2) > 0, "D est monte en ligne 2");
	h_eq_u64("ligne 3 vidée", kfix_cell_fg(&f, 0, 3), 0);
	kfix_close(&f);
}

static void	last_row_is_background(void)
{
	t_kfix	f;
	t_rect	row;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kcon_clear_all(&f.k);
	kfix_feed(&f, "0123456789\n0123456789\n0123456789\n0123456789\n");
	row = rect_make(0, 3 * 16, 80, 16);
	h_eq_u64("derniere ligne entierement a la couleur de fond",
		fake_count(&f.g.s, row, f.k.bg), 80 * 16);
	kfix_feed(&f, "E");
	h_true(kfix_is(&f, 0, 'E', 3000), "E ecrit en derniere ligne");
	kfix_close(&f);
}

static void	many_scrolls(void)
{
	t_kfix	f;
	int		i;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	i = 0;
	while (i < 1000)
	{
		kfix_feed(&f, "ligne\n");
		i++;
	}
	h_eq_i64("ligne apres 1000 defilements", f.k.row, 3);
	kfix_close(&f);
}

static void	single_cell_console(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 8, 16, 8))
		return ;
	h_eq_i64("1 colonne", f.k.cols, 1);
	h_eq_i64("1 ligne", f.k.rows, 1);
	kfix_feed(&f, "ab\ncd");
	h_eq_i64("ligne unique", f.k.row, 0);
	h_eq_i64("colonne apres d", f.k.col, 1);
	h_true(kfix_is(&f, 0, 'd', 0), "d en (0,0)");
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_scroll");
	h_run("scroll un defilement", one_scroll);
	h_run("scroll derniere ligne au fond", last_row_is_background);
	h_run("scroll 1000 defilements", many_scrolls);
	h_run("scroll console d'une cellule", single_cell_console);
	return (h_end());
}
