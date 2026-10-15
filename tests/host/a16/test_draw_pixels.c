#include <stdlib.h>
#include "a16_test.h"

#define SURF_W 160
#define SURF_H 160
#define AT_X 5
#define AT_Y 7

static const char	*g_chars[] = {
	"H", "j", "\xC3\xA9", "\xE2\x82\xAC", "\xC5\x92", "\xFF",
	"\xF0\x9F\x98\x80",
};

static int	expected_at(const t_glyph *g, int32_t pos)
{
	int32_t	dx;
	int32_t	dy;

	dx = pos % SURF_W - AT_X - g->xoff;
	dy = pos / SURF_W - AT_Y - g->yoff;
	if (dx < 0 || dy < 0 || dx >= g->w || dy >= g->h)
		return (0);
	return (glyph_pixel(g, dx, dy));
}

static void	check_exact(t_fontid id, const char *text)
{
	t_surface		s;
	t_textreq		rq;
	const t_glyph	*g;
	const char		*cur;
	int32_t			pos;

	fake_alloc(&s, SURF_W, SURF_H);
	req_set(&rq, id, point_of(AT_X, AT_Y), text);
	cur = text;
	g = font_glyph(rq.font, font_utf8_next(&cur, text + strlen(text)));
	font_draw(&s, &rq);
	h_eq_i64("PIXEL-appels egaux aux pixels allumes", g_fake.calls,
		glyph_ink(g));
	h_eq_i64("PIXEL-aucun appel hors clip", g_fake.outside, 0);
	h_eq_i64("PIXEL-aucun pixel peint deux fois", g_fake.repeated, 0);
	pos = 0;
	while (pos < SURF_W * SURF_H)
	{
		h_true((s.px[pos] == FAKE_INK) == expected_at(g, pos),
			"PIXEL-image egale a la bitmap du glyphe");
		pos++;
	}
	free(s.px);
}

static void	draw_pixels_equal_glyph_bitmap(void)
{
	int		id;
	size_t	i;

	id = 0;
	while (id < FONT_IDS)
	{
		i = 0;
		while (i < sizeof(g_chars) / sizeof(g_chars[0]))
			check_exact((t_fontid)id, g_chars[i++]);
		id++;
	}
}

int	main(void)
{
	h_begin("a16/draw_pixels");
	h_run("pixels egaux a la bitmap du glyphe", draw_pixels_equal_glyph_bitmap);
	return (h_end());
}
