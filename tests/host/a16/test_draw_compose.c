#include <stdio.h>
#include <stdlib.h>
#include "a16_test.h"

static const char	*g_pairs[][2] = {
{"A", "B"}, {"\xC3\xA9", "\xE2\x82\xAC"}, {"W", "j"}, {"f", "i"},
{"\xC3\x80", "\xC3\x89"}, {"l", "'"}, {"T", "."}, {"\xFF", "x"},
};

static const char	*g_broken[][2] = {
{"\xFF", "\xEF\xBF\xBD"}, {"\xE2\x82", "\xEF\xBF\xBD"},
{"\xC0\x80", "\xEF\xBF\xBD\xEF\xBF\xBD"},
{"\xED\xA0\x80", "\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"},
{"a\xF4\x90\x80\x80", "a\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD\xEF\xBF\xBD"},
};

static uint64_t	hash_split(t_fontid id, const char *a, const char *b)
{
	t_surface	s;
	t_textreq	rq;
	uint64_t	h;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, id, point_of(3, 4), a);
	font_draw(&s, &rq);
	rq.at.x += font_text_width(rq.font, a, -1);
	rq.text = b;
	font_draw(&s, &rq);
	h = fake_hash(&s);
	free(s.px);
	return (h);
}

static void	compose_color_passthrough(void)
{
	t_surface	s;
	t_textreq	rq;
	int32_t		pos;
	int			ink;

	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, FONT_UI_BOLD, point_of(3, 4), "H\xC3\xA9");
	rq.color = 0x00112233;
	font_draw(&s, &rq);
	ink = 0;
	pos = 0;
	while (pos < RENDER_W * RENDER_H)
	{
		h_true(s.px[pos] == FAKE_PAPER || s.px[pos] == 0x00112233,
			"COULEUR-valeur transmise telle quelle, alpha compris");
		ink += s.px[pos++] == 0x00112233;
	}
	h_true(ink > 10, "COULEUR-du texte est bien dessine");
	free(s.px);
}

static void	compose_additive_metamorphic(void)
{
	int			id;
	size_t		i;
	t_textreq	rq;
	char		both[16];

	id = 0;
	while (id < FONT_IDS)
	{
		i = 0;
		while (i < sizeof(g_pairs) / sizeof(g_pairs[0]))
		{
			snprintf(both, sizeof(both), "%s%s", g_pairs[i][0], g_pairs[i][1]);
			req_set(&rq, (t_fontid)id, point_of(3, 4), both);
			h_eq_u64("ADD-AB egal A puis B decale de la chasse de A",
				render_hash(&rq), hash_split((t_fontid)id, g_pairs[i][0],
					g_pairs[i][1]));
			i++;
		}
		id++;
	}
}

static void	compose_invalid_is_replacement(void)
{
	int			id;
	size_t		i;
	t_textreq	rq[2];

	id = 0;
	while (id < FONT_IDS)
	{
		i = 0;
		while (i < sizeof(g_broken) / sizeof(g_broken[0]))
		{
			req_set(&rq[0], (t_fontid)id, point_of(3, 4), g_broken[i][0]);
			req_set(&rq[1], (t_fontid)id, point_of(3, 4), g_broken[i][1]);
			h_eq_u64("INVALIDE-sequence mal formee = autant de U+FFFD",
				render_hash(&rq[0]), render_hash(&rq[1]));
			i++;
		}
		id++;
	}
}

int	main(void)
{
	h_begin("a16/draw_compose");
	h_run("couleur transmise a gfx_put", compose_color_passthrough);
	h_run("additivite : AB = A puis B", compose_additive_metamorphic);
	h_run("utf-8 invalide = U+FFFD", compose_invalid_is_replacement);
	return (h_end());
}
