#include <stdlib.h>
#include "a16_test.h"
#include "velum/gfx.h"

#define AT_X 4
#define AT_Y 3
#define INK 0xFF102030u

static const t_rect	g_clips[] = {
{0, 0, RENDER_W, RENDER_H}, {10, 5, 20, 12}, {-5, -5, 20, 20},
{40, 20, 100, 100}, {0, 0, 0, 0}, {20, 8, 1, 1},
};

static int	in_clip(const t_rect *clip, int32_t pos)
{
	t_point	p;

	p = point_of(pos % RENDER_W, pos / RENDER_W);
	return (rect_contains(*clip, p));
}

static void	verify_pixels(const t_surface *s, const uint8_t *ink)
{
	int32_t		pos;
	uint32_t	want;

	pos = 0;
	while (pos < RENDER_W * RENDER_H)
	{
		want = 0xFFFFFFFFu;
		if (ink[pos] && in_clip(&s->clip, pos))
			want = INK;
		h_true(s->px[pos] == want, "INTEG-pixel egal au modele, vrai gfx_put");
		pos++;
	}
}

static void	check_surface(t_fontid id, t_rect clip)
{
	t_surface	s;
	t_textreq	rq;
	uint8_t		*ink;

	s.px = malloc(RENDER_W * RENDER_H * sizeof(uint32_t));
	ink = calloc(RENDER_W * RENDER_H, 1);
	if (!s.px || !ink)
		abort();
	gfx_surface_init(&s, s.px, RENDER_W, RENDER_H);
	gfx_fill(&s, rect_make(0, 0, RENDER_W, RENDER_H), 0xFFFFFFFFu);
	gfx_set_clip(&s, clip);
	req_set(&rq, id, point_of(AT_X, AT_Y), "H\xC3\xA9\xE2\x82\xAC j\xFF");
	rq.color = INK;
	font_draw(&s, &rq);
	ink_of(rq.font, rq.text, point_of(AT_X, AT_Y), ink);
	verify_pixels(&s, ink);
	free(s.px);
	free(ink);
}

static void	integ_every_font_and_clip(void)
{
	int		id;
	size_t	i;

	id = 0;
	while (id < FONT_IDS)
	{
		i = 0;
		while (i < sizeof(g_clips) / sizeof(g_clips[0]))
			check_surface((t_fontid)id, g_clips[i++]);
		id++;
	}
}

int	main(void)
{
	h_begin("a16/integ_gfx");
	h_run("rendu contre le vrai gfx_put", integ_every_font_and_clip);
	return (h_end());
}
