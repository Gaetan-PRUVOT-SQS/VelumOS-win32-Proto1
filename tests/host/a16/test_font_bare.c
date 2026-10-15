#include <stdlib.h>
#include "a16_test.h"

static void	bare_width_and_fit(void)
{
	const t_font	*f;

	f = bare_font();
	h_eq_i64("NU-largeur : absent compte 0", font_text_width(f, "xAy", -1), 5);
	h_eq_i64("NU-largeur : glyphes sans encre", font_text_width(f, "ABC", -1),
		13);
	h_eq_i64("NU-coupe : absent passe", font_fit(f, "xAy", 5), 3);
	h_eq_i64("NU-coupe : deuxieme A hors largeur", font_fit(f, "xAA", 5), 2);
	h_eq_i64("NU-coupe : tout tient", font_fit(f, "xAA", 10), 3);
	h_true(font_glyph(f, 0x78) == NULL, "NU-sans U+FFFD, absent donne NULL");
}

static void	check_calls(const char *text, int32_t want)
{
	t_surface	s;
	t_textreq	rq;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI, point_of(3, 4), text);
	rq.font = bare_font();
	font_draw(&s, &rq);
	h_eq_i64("NU-appels de gfx_put", g_fake.calls, want);
	h_eq_i64("NU-aucun appel hors surface", g_fake.outside, 0);
	free(s.px);
}

static void	bare_draw_degenerate_glyphs(void)
{
	check_calls("xAy", 3);
	check_calls("B", 0);
	check_calls("C", 0);
	check_calls("ABC", 3);
	check_calls("xyz", 0);
}

int	main(void)
{
	h_begin("a16/font_bare");
	h_run("police sans U+FFFD : largeur et coupe", bare_width_and_fit);
	h_run("glyphes absents ou sans encre", bare_draw_degenerate_glyphs);
	return (h_end());
}
