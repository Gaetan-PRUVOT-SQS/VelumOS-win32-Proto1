#include <stdio.h>
#include "a16_test.h"

#define BUDGET_BYTES 81920

static const uint8_t	g_mono_h[9] = {0x84, 0x84, 0x84, 0x84, 0xFC, 0x84,
	0x84, 0x84, 0x84};

static int64_t	blob_size(const t_font *f)
{
	return (f->nbits);
}

static void	budget_under_80_kib(void)
{
	int				id;
	int64_t			total;
	int64_t			blob;
	int64_t			entry;
	const t_font	*f;

	entry = sizeof(t_glyph);
	id = 0;
	total = 0;
	blob = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		total += f->nglyphs * entry;
		total += sizeof(t_font);
		blob += blob_size(f);
	}
	printf("a16/font_budget : %lld octets (tables %lld, bitmaps %lld)\n",
		(long long)(total + blob), (long long)total, (long long)blob);
	h_true(total + blob < BUDGET_BYTES, "BUDGET-4 polices sous 80 Kio");
}

static void	pinned_mono_h_after_review(void)
{
	const t_glyph	*g;
	const uint8_t	*rows;
	int				row;

	g = font_glyph(font_get(FONT_MONO), 'H');
	rows = g_mono_h + 1;
	h_true(font_glyph_rows(font_get(FONT_MONO), g, &rows),
		"EPINGLE-H mono dans la table");
	h_eq_i64("EPINGLE-H mono largeur", g->w, 6);
	h_eq_i64("EPINGLE-H mono hauteur", g->h, 9);
	h_eq_i64("EPINGLE-H mono xoff", g->xoff, 1);
	h_eq_i64("EPINGLE-H mono yoff", g->yoff, 4);
	h_eq_i64("EPINGLE-H mono chasse", g->advance, 8);
	row = 0;
	while (row < 9)
	{
		h_eq_u64("EPINGLE-H mono rangee", rows[row], g_mono_h[row]);
		row++;
	}
}

static void	baseline_is_shared_by_flat_letters(void)
{
	const char		*flat;
	int				id;
	int				i;
	const t_font	*f;
	const t_glyph	*g;

	flat = "HIEFLTxzvwkn";
	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		i = 0;
		while (flat[i])
		{
			g = font_glyph(f, (uint32_t)flat[i++]);
			h_eq_i64("LIGNE-pied du glyphe sur ascent", g->yoff + g->h,
				f->ascent);
		}
	}
}

int	main(void)
{
	h_begin("a16/font_budget");
	h_run("budget de taille", budget_under_80_kib);
	h_run("glyphe epingle apres relecture", pinned_mono_h_after_review);
	h_run("ligne de base commune", baseline_is_shared_by_flat_letters);
	return (h_end());
}
