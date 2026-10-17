#include "a16_test.h"

static void	charset_sizes_match_the_brief(void)
{
	h_eq_i64("JEU-french 128 glyphes", charset_count(CS_FRENCH), 128);
	h_eq_i64("JEU-display 122 glyphes", charset_count(CS_DISPLAY), 122);
	h_eq_i64("JEU-digits 14 glyphes", charset_count(CS_DIGITS), 14);
}

static void	check_membership(const t_font *f, t_cset set)
{
	int32_t	k;

	k = 0;
	while (k < f->nglyphs)
	{
		h_true(charset_has(set, f->glyphs[k].cp),
			"JEU-chaque glyphe appartient au jeu de sa police");
		k++;
	}
}

static void	check_ascii_direct(const t_font *f, t_cset set)
{
	int32_t	k;

	if (set == CS_DIGITS)
		return ;
	k = 0;
	while (k <= 0x7E - 0x20)
	{
		h_eq_u64("JEU-ascii a l'indice cp - 0x20", f->glyphs[k].cp,
			(uint32_t)(0x20 + k));
		k++;
	}
}

static void	charset_per_font(void)
{
	int	id;

	id = 0;
	while (id < FONT_IDS)
	{
		check_membership(font_get((t_fontid)id), charset_of((t_fontid)id));
		check_ascii_direct(font_get((t_fontid)id), charset_of((t_fontid)id));
		id++;
	}
}

int	main(void)
{
	h_begin("a16/font_charset");
	h_run("effectifs des jeux de la fiche", charset_sizes_match_the_brief);
	h_run("glyphes de chaque police dans son jeu", charset_per_font);
	return (h_end());
}
