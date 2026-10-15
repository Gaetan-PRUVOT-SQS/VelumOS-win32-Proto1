#include "kfix.h"
#include "velum/err.h"

static void	setup_refusals(void)
{
	t_gsurf	g;
	t_kcon	k;

	fake_reset();
	fake_gsurf_new(&g, 80, 64, 80);
	h_eq_i64("surface nulle", kcon_setup(&k, NULL, font_get(FONT_MONO)),
		E_INVAL);
	g.s.px = NULL;
	h_eq_i64("pixels nuls", kfix_try(&g.s), E_INVAL);
	g.s.px = g.px;
	g.s.stride = 79;
	h_eq_i64("pas < largeur", kfix_try(&g.s), E_INVAL);
	g.s.stride = 80;
	h_eq_i64("police absente", kcon_setup(&k, &g.s, NULL), E_INVAL);
	h_true(!k.ready, "console non prete apres refus");
	fake_gsurf_free(&g);
}

static void	cell_limits(void)
{
	t_gsurf	g;

	fake_reset();
	fake_gsurf_new(&g, 80, 64, 80);
	g_ffont.cell_w = 65;
	h_eq_i64("cellule trop large", kfix_try(&g.s), E_INVAL);
	g_ffont.cell_w = 0;
	h_eq_i64("cellule de largeur nulle", kfix_try(&g.s), E_INVAL);
	g_ffont.cell_w = 8;
	g_ffont.font.height = 0;
	h_eq_i64("hauteur nulle", kfix_try(&g.s), E_INVAL);
	g_ffont.font.height = 65;
	h_eq_i64("hauteur 65", kfix_try(&g.s), E_INVAL);
	fake_gsurf_free(&g);
}

static void	smaller_than_cell(void)
{
	t_gsurf	g;
	t_kcon	k;

	fake_reset();
	fake_gsurf_new(&g, 7, 16, 7);
	h_eq_i64("largeur 7 < cellule", kfix_try(&g.s), E_RANGE);
	fake_gsurf_free(&g);
	fake_gsurf_new(&g, 8, 15, 8);
	h_eq_i64("hauteur 15 < cellule", kfix_try(&g.s), E_RANGE);
	fake_gsurf_free(&g);
	fake_gsurf_new(&g, 15, 31, 15);
	h_eq_i64("15x31 : 1 colonne", kcon_setup(&k, &g.s, font_get(FONT_MONO)),
		E_OK);
	h_eq_i64("15x31 : cols", k.cols, 1);
	h_eq_i64("15x31 : rows", k.rows, 1);
	fake_gsurf_free(&g);
}

static void	padded_stride(void)
{
	t_kfix	f;
	int32_t	y;
	int32_t	x;
	int		bad;

	if (!kfix_open(&f, 80, 64, 96))
		return ;
	kfix_feed(&f, "0123456789\n0123456789\n0123456789\n0123456789\n0123456789");
	bad = 0;
	y = 0;
	while (y < 64)
	{
		x = 80;
		while (x < 96)
			bad += (f.g.px[y * 96 + x++] != 0);
		y++;
	}
	h_eq_i64("marge de pas jamais ecrite", bad, 0);
	kfix_close(&f);
}

int	main(void)
{
	h_begin("a13/kcon_geom");
	h_run("geometrie refus", setup_refusals);
	h_run("geometrie limites de cellule", cell_limits);
	h_run("geometrie surface < cellule", smaller_than_cell);
	h_run("geometrie pas de ligne > largeur", padded_stride);
	return (h_end());
}
