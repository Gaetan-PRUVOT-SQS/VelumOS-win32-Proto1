#include "a16_test.h"

static void	check_padding(const t_glyph *g)
{
	int32_t	row;
	int32_t	col;

	row = 0;
	while (row < g->h)
	{
		col = g->w;
		while (col < (g->w + 7) / 8 * 8)
		{
			h_true(glyph_pixel(g, col, row) == 0,
				"BITS-bits de bourrage de fin de rangee a zero");
			col++;
		}
		row++;
	}
}

static void	check_tight_box(const t_glyph *g)
{
	int32_t	row;
	int32_t	col;
	int		edges;

	edges = 0;
	row = 0;
	while (row < g->h)
	{
		col = 0;
		while (col < g->w)
		{
			if (glyph_pixel(g, col, row))
				edges |= (row == 0) | ((row == g->h - 1) << 1)
					| ((col == 0) << 2) | ((col == g->w - 1) << 3);
			col++;
		}
		row++;
	}
	h_eq_i64("BITS-boite serree : encre sur les 4 bords", edges, 15);
}

static void	bits_padding_and_tight_box(void)
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
		{
			if (f->glyphs[k].w > 0)
			{
				check_padding(&f->glyphs[k]);
				check_tight_box(&f->glyphs[k]);
			}
			else
				h_true(f->glyphs[k].cp == 0x20 || f->glyphs[k].cp == 0xA0,
					"BITS-seuls espace et espace insecable sont vides");
			k++;
		}
	}
}

int	main(void)
{
	h_begin("a16/font_bits");
	h_run("bourrage a zero et boite serree", bits_padding_and_tight_box);
	return (h_end());
}
