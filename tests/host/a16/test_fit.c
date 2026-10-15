#include <stdlib.h>
#include "a16_test.h"

static const char	*g_phrases[] = {
	"Démarrer", "Poste de travail", "Éteindre l'ordinateur",
	"€ œ Œ « » … – —", "😀x日本", "a\xFFz\xE2\x82", "ÀÂÇÉÈ àâçéè",
};

static size_t	check_cut(const t_font *f, const char *s, int32_t max_w)
{
	size_t	len;
	size_t	n;

	len = strlen(s);
	n = (size_t)font_fit(f, s, max_w);
	h_true(n <= len, "COUPE-jamais au-dela de la chaine");
	h_true(model_is_boundary(s, len, n), "COUPE-frontiere de caractere");
	h_true(model_width(f, s, n) <= max_w || max_w < 0 || n == 0,
		"COUPE-le prefixe tient dans max_w");
	if (n < len && max_w >= 0)
		h_true(model_width(f, s, model_boundary(s, len, n)) > max_w,
			"COUPE-maximal");
	if (max_w < 0)
		h_eq_u64("COUPE-largeur negative donne 0", n, 0);
	return (n);
}

static void	run_phrase(const t_font *f, const char *s)
{
	int32_t	total;
	int32_t	max_w;
	size_t	prev;
	size_t	n;

	total = font_text_width(f, s, -1);
	prev = 0;
	max_w = -2;
	while (max_w <= total + 3)
	{
		n = check_cut(f, s, max_w);
		h_true(n >= prev, "COUPE-croissante avec max_w");
		prev = n;
		max_w++;
	}
	h_eq_i64("COUPE-tout tient", font_fit(f, s, total), (int64_t)strlen(s));
}

static void	fit_every_cut_of_phrases(void)
{
	int		id;
	size_t	i;

	id = 0;
	while (id < FONT_IDS)
	{
		i = 0;
		while (i < sizeof(g_phrases) / sizeof(g_phrases[0]))
			run_phrase(font_get((t_fontid)id), g_phrases[i++]);
		id++;
	}
}

static void	fit_degenerate_inputs(void)
{
	const t_font	*f;
	char			*text;

	f = font_get(FONT_UI);
	h_eq_i64("FIT-police nulle", font_fit(NULL, "abc", 100), 0);
	h_eq_i64("FIT-texte nul", font_fit(f, NULL, 100), 0);
	h_eq_i64("FIT-chaine vide", font_fit(f, "", 100), 0);
	h_eq_i64("FIT-largeur zero", font_fit(f, "abc", 0), 0);
	h_eq_i64("FIT-INT32_MAX", font_fit(f, "abc", INT32_MAX), 3);
	h_eq_i64("FIT-INT32_MIN", font_fit(f, "abc", INT32_MIN), 0);
	text = repeat_char('A', 9000000);
	h_eq_i64("FIT-9 000 000 x 255 : floor(INT32_MAX / 255) caracteres",
		font_fit(wide_font(), text, INT32_MAX), 8421504);
	free(text);
}

int	main(void)
{
	h_begin("a16/fit");
	h_run("toutes les coupes d'une phrase", fit_every_cut_of_phrases);
	h_run("entrees degenerees", fit_degenerate_inputs);
	return (h_end());
}
