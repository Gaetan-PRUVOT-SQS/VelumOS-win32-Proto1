#include "a16_test.h"

static void	check_cell(const t_font *f, const t_glyph *g)
{
	const uint8_t	*rows;

	h_true(g->advance >= 1, "FORME-chasse >= 1");
	h_true((g->w == 0) == (g->h == 0), "FORME-largeur nulle ssi hauteur nulle");
	h_true((g->w == 0) == !font_glyph_rows(f, g, &rows),
		"FORME-bitmap dans la table ssi encre");
	if (g->h == 0)
		return ;
	h_true(g->yoff >= 0, "FORME-glyphe sous le haut de la cellule");
	h_true(g->yoff + g->h <= f->height, "FORME-glyphe dans la cellule");
	h_true(g->xoff >= -4, "FORME-debordement a gauche borne");
	h_true(g->xoff + g->w <= g->advance + 4, "FORME-debord a droite borne");
}

static void	shape_cell_of_every_glyph(void)
{
	int				id;
	int32_t			k;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		k = 0;
		while (k < f->nglyphs)
			check_cell(f, &f->glyphs[k++]);
	}
}

static void	shape_mono_is_8_by_16(void)
{
	const t_font	*f;
	int32_t			k;

	f = font_get(FONT_MONO);
	h_eq_i64("MONO-hauteur de cellule 16", f->height, 16);
	k = 0;
	while (k < f->nglyphs)
	{
		h_eq_i64("MONO-chasse fixe 8", f->glyphs[k].advance, 8);
		h_true(f->glyphs[k].xoff >= 0, "MONO-encre dans la cellule : a gauche");
		h_true(f->glyphs[k].xoff + f->glyphs[k].w <= 8,
			"MONO-encre dans la cellule : a droite");
		k++;
	}
}

static void	shape_replacement_is_a_box(void)
{
	int				id;
	const t_glyph	*g;
	int32_t			row;
	int32_t			col;

	id = 0;
	while (id < FONT_IDS)
	{
		g = font_glyph(font_get((t_fontid)id++), 0xFFFD);
		row = 0;
		while (row < g->h)
		{
			col = 0;
			while (col < g->w)
			{
				h_eq_i64("REMPL-contour seulement", glyph_pixel(g, col, row),
					row == 0 || row == g->h - 1 || col == 0 || col == g->w - 1);
				col++;
			}
			row++;
		}
	}
}

int	main(void)
{
	h_begin("a16/font_shape");
	h_run("cellule de chaque glyphe", shape_cell_of_every_glyph);
	h_run("police mono 8x16", shape_mono_is_8_by_16);
	h_run("glyphe de remplacement en boite", shape_replacement_is_a_box);
	return (h_end());
}
