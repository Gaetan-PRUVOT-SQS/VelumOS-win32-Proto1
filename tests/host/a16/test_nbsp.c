#include <stdlib.h>
#include "a16_test.h"

static int32_t	calls_of(const t_font *f, const char *text)
{
	t_surface	s;
	t_textreq	rq;
	int32_t		calls;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI, point_of(3, 4), text);
	rq.font = f;
	font_draw(&s, &rq);
	calls = g_fake.calls;
	free(s.px);
	return (calls);
}

static void	nbsp_has_the_width_of_a_space(void)
{
	int				id;
	const t_font	*f;
	int32_t			want;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		want = font_text_width(f, "a :", -1);
		h_eq_i64("INSEC-U+00A0 large comme l'espace",
			font_text_width(f, "a\xC2\xA0:", -1), want);
		h_eq_i64("INSEC-U+202F large comme l'espace",
			font_text_width(f, "a\xE2\x80\xAF:", -1), want);
		h_eq_i64("INSEC-U+00A0 seule", font_text_width(f, "\xC2\xA0", -1),
			font_glyph(f, 0x20)->advance);
		h_eq_i64("INSEC-U+202F seule", font_text_width(f, "\xE2\x80\xAF", -1),
			font_glyph(f, 0x20)->advance);
	}
}

static void	nbsp_is_never_the_replacement_box(void)
{
	int				id;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		h_true(font_glyph(f, 0xA0)->cp != 0xFFFD, "INSEC-U+00A0 sans boite");
		h_true(font_glyph(f, 0x202F)->cp != 0xFFFD, "INSEC-U+202F sans boite");
		h_eq_i64("INSEC-U+00A0 sans encre", font_glyph(f, 0xA0)->w, 0);
		h_eq_i64("INSEC-U+202F sans encre", font_glyph(f, 0x202F)->w, 0);
		h_true(font_glyph(f, 0x2030)->cp == 0xFFFD, "INSEC-voisin en boite");
	}
}

static void	nbsp_draws_no_pixel(void)
{
	int				id;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		h_eq_i64("INSEC-U+00A0 aucun pixel", calls_of(f, "\xC2\xA0"), 0);
		h_eq_i64("INSEC-U+202F aucun pixel", calls_of(f, "\xE2\x80\xAF"), 0);
		h_eq_i64("INSEC-meme encre avec U+00A0", calls_of(f, "a\xC2\xA0:"),
			calls_of(f, "a :"));
		h_eq_i64("INSEC-meme encre avec U+202F",
			calls_of(f, "a\xE2\x80\xAF:"), calls_of(f, "a :"));
		h_true(calls_of(f, "\xE2\x80\xB0") > 0, "INSEC-inconnu en boite");
	}
}

int	main(void)
{
	h_begin("a16/nbsp");
	h_run("espaces insecables : largeur", nbsp_has_the_width_of_a_space);
	h_run("espaces insecables : glyphe", nbsp_is_never_the_replacement_box);
	h_run("espaces insecables : dessin", nbsp_draws_no_pixel);
	return (h_end());
}
