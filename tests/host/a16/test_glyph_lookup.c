#include "a16_test.h"

static const t_glyph	g_tiny_glyphs[] = {
{0x20, 0, 0, 0, 0, 3, 0},
{0x41, 0, 0, 0, 0, 5, 0},
{0x5A, 0, 0, 0, 0, 6, 0},
{0x100, 0, 0, 0, 0, 7, 0},
{0x2000, 0, 0, 0, 0, 8, 0},
{0xFFFD, 0, 0, 0, 0, 9, 0},
};

static const t_glyph	g_sparse_glyphs[] = {
{0x20, 0, 0, 0, 0, 3, 0},
{0x41, 0, 0, 0, 0, 5, 0},
{0x42, 0, 0, 0, 0, 6, 0},
{0xFFFD, 0, 0, 0, 0, 9, 0},
};

static const uint32_t	g_absent[] = {
	0, 0x1F, 0x21, 0x26, 0x40, 0x42, 0x59, 0x5B, 0xFF, 0x101, 0x1FFF,
	0x2001, 0xFFFC, 0xFFFE, 0x10FFFF, 0xD800, 0xFFFFFFFF,
};

static const t_font		g_tiny = {8, 2, 10, 6, g_tiny_glyphs,
	NULL, 0};
static const t_font		g_sparse = {8, 2, 10, 4, g_sparse_glyphs,
	NULL, 0};

static void	lookup_present_and_absent(void)
{
	size_t	i;

	i = 0;
	while (i < 6)
	{
		h_true(font_glyph(&g_tiny, g_tiny_glyphs[i].cp) == &g_tiny_glyphs[i],
			"RECH-glyphe present (premier, dernier, milieu)");
		i++;
	}
	i = 0;
	while (i < sizeof(g_absent) / sizeof(g_absent[0]))
	{
		h_true(font_glyph(&g_tiny, g_absent[i]) == &g_tiny_glyphs[5],
			"RECH-absent donne le glyphe de remplacement");
		i++;
	}
}

static void	lookup_ascii_direct_guard(void)
{
	h_true(font_glyph(&g_sparse, 0x41) == &g_sparse_glyphs[1],
		"DIRECT-garde 'A' : indice 33 hors table, repli sur la recherche");
	h_true(font_glyph(&g_sparse, 0x42) == &g_sparse_glyphs[2],
		"DIRECT-garde 'B'");
	h_true(font_glyph(&g_sparse, 0x21) == &g_sparse_glyphs[3],
		"DIRECT-garde '!' : indice 1 a un autre point de code");
	h_true(font_glyph(&g_sparse, 0x20) == &g_sparse_glyphs[0],
		"DIRECT-espace a l'indice 0");
	h_true(font_glyph(&g_sparse, 0x24) == &g_sparse_glyphs[3],
		"DIRECT-garde : indice egal a nglyphs, pas de lecture hors table");
}

static void	lookup_degenerate_fonts(void)
{
	t_font	broken;

	broken = g_tiny;
	h_true(font_glyph(NULL, 0x41) == NULL, "DEGENERE-police nulle");
	broken.nglyphs = 0;
	h_true(font_glyph(&broken, 0x41) == NULL, "DEGENERE-aucun glyphe");
	broken.nglyphs = -3;
	h_true(font_glyph(&broken, 0x41) == NULL, "DEGENERE-compte negatif");
	broken = g_tiny;
	broken.glyphs = NULL;
	h_true(font_glyph(&broken, 0x41) == NULL, "DEGENERE-table nulle");
	broken = g_tiny;
	broken.nglyphs = 5;
	h_true(font_glyph(&broken, 0x42) == NULL, "DEGENERE-sans U+FFFD");
	broken.nglyphs = 1;
	h_true(font_glyph(&broken, 0x20) == &g_tiny_glyphs[0],
		"DEGENERE-un seul glyphe present");
	h_true(font_glyph(&broken, 0x41) == NULL, "DEGENERE-un seul absent");
}

static void	lookup_real_fonts_against_scan(void)
{
	int				id;
	uint32_t		cp;
	const t_font	*f;
	int32_t			k;
	const t_glyph	*want;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		cp = 0;
		while (cp < 0x2200)
		{
			want = &f->glyphs[(f->nglyphs - 1) * (cp != 0x202F)];
			k = 0;
			while (k < f->nglyphs && f->glyphs[k].cp != cp)
				k++;
			if (k < f->nglyphs)
				want = &f->glyphs[k];
			h_true(font_glyph(f, cp) == want, "RECH-police reelle vs balayage");
			cp++;
		}
		h_true(font_glyph(f, 0x10FFFF) == &f->glyphs[f->nglyphs - 1],
			"RECH-hors plage");
	}
}

int	main(void)
{
	h_begin("a16/glyph_lookup");
	h_run("present et absent", lookup_present_and_absent);
	h_run("garde de la table directe ascii", lookup_ascii_direct_guard);
	h_run("polices degenerees", lookup_degenerate_fonts);
	h_run("polices reelles vs balayage", lookup_real_fonts_against_scan);
	return (h_end());
}
