#include <stdlib.h>
#include "a16_test.h"

static const t_rect		g_clips[] = {
{0, 0, RENDER_W, RENDER_H}, {0, 0, 48, RENDER_H}, {48, 0, 48, RENDER_H},
{0, 10, RENDER_W, 3}, {20, 12, 1, 1}, {-50, -50, 400, 400}, {70, 0, 100, 100},
{-10, -10, 20, 20}, {30, 5, 0, 20}, {30, 5, 20, -4}, {95, 39, 5, 5},
};

static const t_point	g_places[] = {
{INT32_MIN, INT32_MIN}, {INT32_MAX, INT32_MAX}, {INT32_MAX - 3, 5},
{-1000000, 5}, {5, INT32_MAX}, {5, INT32_MIN}, {INT32_MIN, 0}, {-40, -6},
};

static int	in_rect(const t_rect *r, int32_t pos)
{
	int64_t	x;
	int64_t	y;

	x = pos % RENDER_W;
	y = pos / RENDER_W;
	return (x >= r->x && y >= r->y && x < (int64_t)r->x + r->w
		&& y < (int64_t)r->y + r->h);
}

static void	check_clip(const t_rect *clip, const char *text)
{
	t_surface	plain;
	t_surface	cut;
	t_textreq	rq;
	int32_t		pos;
	uint32_t	want;

	fake_alloc(&plain, RENDER_W, RENDER_H);
	req_set(&rq, FONT_MONO, point_of(-3, 2), text);
	font_draw(&plain, &rq);
	fake_alloc(&cut, RENDER_W, RENDER_H);
	cut.clip = *clip;
	font_draw(&cut, &rq);
	h_eq_i64("CLIP-aucun appel hors clip ni hors surface", g_fake.outside, 0);
	pos = 0;
	while (pos < RENDER_W * RENDER_H)
	{
		want = FAKE_PAPER;
		if (in_rect(clip, pos))
			want = plain.px[pos];
		h_true(cut.px[pos] == want, "CLIP-image = image sans clip masquee");
		pos++;
	}
	free(plain.px);
	free(cut.px);
}

static void	clip_partitions_against_unclipped(void)
{
	size_t		i;
	const char	*text;

	text = "Poste de travail \xC3\x89teindre \xE2\x82\xAC";
	i = 0;
	while (i < sizeof(g_clips) / sizeof(g_clips[0]))
		check_clip(&g_clips[i++], text);
}

static void	clip_extreme_positions(void)
{
	size_t		i;
	t_surface	s;
	t_textreq	rq;

	fake_alloc(&s, RENDER_W, RENDER_H);
	i = 0;
	while (i < sizeof(g_places) / sizeof(g_places[0]))
	{
		req_set(&rq, FONT_UI, g_places[i++], "D\xC3\xA9marrer \xE2\x82\xAC");
		font_draw(&s, &rq);
	}
	h_eq_i64("EXTREME-aucun appel hors surface", g_fake.outside, 0);
	free(s.px);
}

int	main(void)
{
	h_begin("a16/draw_clip");
	h_run("partitions de clip contre le rendu sans clip",
		clip_partitions_against_unclipped);
	h_run("positions extremes sans debordement", clip_extreme_positions);
	return (h_end());
}
