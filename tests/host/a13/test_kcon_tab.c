#include "kfix.h"

static void	from_start_and_seven(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "\t");
	h_eq_i64("tab depuis la colonne 0", f.k.col, 8);
	h_eq_u64("huit cellules vides", kfix_spaces(), 8);
	kfix_feed(&f, "\ra");
	kfix_feed(&f, "bcdefg\t");
	h_eq_i64("tab depuis la colonne 7", f.k.col, 8);
	kfix_close(&f);
}

static void	clamped_at_edge(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "abcdefgh\t");
	h_eq_i64("tab depuis 8 plafonne a cols", f.k.col, 10);
	h_eq_i64("meme ligne", f.k.row, 0);
	kfix_feed(&f, "\r012345678\t");
	h_eq_i64("tab depuis cols-1", f.k.col, 10);
	kfix_close(&f);
}

static void	pending_wrap(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "0123456789\t");
	h_eq_i64("tab apres derniere colonne : ligne suivante", f.k.row, 1);
	h_eq_i64("tab apres derniere colonne : colonne 8", f.k.col, 8);
	kfix_close(&f);
}

static void	tab_clears_cells(void)
{
	t_kfix	f;

	if (!kfix_open(&f, 80, 64, 80))
		return ;
	kfix_feed(&f, "abcdefgh");
	h_true(kfix_cell_fg(&f, 0, 0) > 0, "a visible");
	kfix_feed(&f, "\r\t");
	h_eq_u64("tab efface la cellule 0", kfix_cell_fg(&f, 0, 0), 0);
	h_eq_u64("tab efface la cellule 7", kfix_cell_fg(&f, 7, 0), 0);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_tab");
	h_run("tab depuis 0 et 7", from_start_and_seven);
	h_run("tab plafonnee au bord", clamped_at_edge);
	h_run("tab apres la derniere colonne", pending_wrap);
	h_run("tab efface les cellules", tab_clears_cells);
	return (h_end());
}
