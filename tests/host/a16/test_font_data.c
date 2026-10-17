#include "a16_test.h"

static void	data_font_get_ids(void)
{
	int				id;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id);
		h_true(f != NULL, "ID-police connue");
		h_true(f != font_get((t_fontid)(id + 1)) || id + 1 == FONT_IDS,
			"ID-polices distinctes");
		h_true(f->ascent > 0 && f->descent > 0, "ID-ascent, descent > 0");
		h_eq_i64("ID-hauteur = ascent + descent", f->height,
			f->ascent + f->descent);
		id++;
	}
	h_true(font_get(FONT_IDS) == NULL, "ID-borne haute exclue");
	h_true(font_get((t_fontid) - 1) == NULL, "ID-negatif");
	h_true(font_get((t_fontid)77) == NULL, "ID-hors enumeration");
}

static void	check_table(const t_font *f, t_cset set)
{
	int32_t	k;

	h_eq_i64("TABLE-nombre de glyphes du jeu", f->nglyphs,
		charset_count(set));
	h_eq_u64("TABLE-premier = espace", f->glyphs[0].cp, 0x20);
	h_eq_u64("TABLE-dernier = U+FFFD", f->glyphs[f->nglyphs - 1].cp, 0xFFFD);
	k = 1;
	while (k < f->nglyphs)
	{
		h_true(f->glyphs[k - 1].cp < f->glyphs[k].cp,
			"TABLE-points de code strictement croissants");
		k++;
	}
}

static void	data_sorted_and_sized(void)
{
	int	id;

	id = 0;
	while (id < FONT_IDS)
	{
		check_table(font_get((t_fontid)id), charset_of((t_fontid)id));
		id++;
	}
}

static void	data_kernel_selftest_passes(void)
{
	h_eq_i64("AUTOTEST-font_selftest renvoie 0", font_selftest(), 0);
}

int	main(void)
{
	h_begin("a16/font_data");
	h_run("identifiants et metriques", data_font_get_ids);
	h_run("tri et effectif de la table par jeu", data_sorted_and_sized);
	h_run("autotest noyau", data_kernel_selftest_passes);
	return (h_end());
}
