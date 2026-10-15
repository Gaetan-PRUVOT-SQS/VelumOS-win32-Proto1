#include <stdlib.h>
#include "a16_test.h"

static const char	*g_phrases[] = {
	"", "A", "Démarrer", "Poste de travail", "Éteindre l'ordinateur",
	"€ œ Œ « » … – — ‘ ’ “ ”", "ÀÂÇÉÈ àâçéè", "😀x", "\xE2\x82", "a\xFFz",
	"日本語", "\xC0\x80\xED\xA0\x80",
};

static void	width_equals_advance_sum(void)
{
	int				id;
	size_t			i;
	size_t			len;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		i = 0;
		while (i < sizeof(g_phrases) / sizeof(g_phrases[0]))
		{
			len = strlen(g_phrases[i]);
			h_eq_i64("LARGEUR-somme des chasses", font_text_width(f,
					g_phrases[i], (int32_t)len),
				model_width(f, g_phrases[i], len));
			h_eq_i64("LARGEUR-len negatif = jusqu'au NUL", font_text_width(f,
					g_phrases[i], -1), model_width(f, g_phrases[i], len));
			i++;
		}
	}
	h_eq_i64("LARGEUR-mono : 8 par caractere", font_text_width(font_get(
				FONT_MONO), "Démarrer", 9), 64);
}

static void	width_len_semantics(void)
{
	const t_font	*f;

	f = font_get(FONT_UI);
	h_eq_i64("LEN-zero", font_text_width(f, "abc", 0), 0);
	h_eq_i64("LEN-INT32_MIN", font_text_width(f, "ab", INT32_MIN),
		font_text_width(f, "ab", 2));
	h_eq_i64("LEN-plus long que la chaine, arret au NUL",
		font_text_width(f, "ab\0cd", 5), font_text_width(f, "ab", 2));
	h_eq_i64("LEN-coupe au milieu d'un caractere",
		font_text_width(f, "\xC3\xA9", 1), font_glyph(f, 0xFFFD)->advance);
	h_eq_i64("LEN-caractere entier", font_text_width(f, "\xC3\xA9", 2),
		font_glyph(f, 0xE9)->advance);
	h_eq_i64("LEN-INT32_MAX borne par le NUL", font_text_width(f, "ab",
			INT32_MAX), font_text_width(f, "ab", 2));
}

static void	width_null_and_saturation(void)
{
	char	*text;

	h_eq_i64("NUL-police nulle", font_text_width(NULL, "abc", 3), 0);
	h_eq_i64("NUL-texte nul", font_text_width(font_get(FONT_UI), NULL, 3), 0);
	text = repeat_char('A', 8000000);
	h_eq_i64("SATURATION-8 000 000 x 255 sous INT32_MAX", font_text_width(
			wide_font(), text, -1), 2040000000);
	free(text);
	text = repeat_char('A', 9000000);
	h_eq_i64("SATURATION-9 000 000 x 255 plafonne a INT32_MAX",
		font_text_width(wide_font(), text, -1), INT32_MAX);
	free(text);
}

int	main(void)
{
	h_begin("a16/width");
	h_run("largeur = somme des chasses", width_equals_advance_sum);
	h_run("semantique de len", width_len_semantics);
	h_run("pointeurs nuls et saturation", width_null_and_saturation);
	return (h_end());
}
