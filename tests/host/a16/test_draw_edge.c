#include <stdlib.h>
#include "a16_test.h"

static int32_t	visible_ink(const t_glyph *g)
{
	int32_t	count;
	int32_t	row;
	int32_t	col;

	count = 0;
	row = 0;
	while (row < g->h)
	{
		col = 0;
		while (col < g->w && col < -g->xoff)
			count += glyph_pixel(g, col++, row);
		row++;
	}
	return (count);
}

static void	check_glyph(t_fontid id, const t_glyph *g)
{
	t_surface	s;
	t_textreq	rq;
	uint8_t		enc[5];

	enc[ref_utf8_encode(g->cp, enc)] = '\0';
	fake_alloc(&s, RENDER_W, RENDER_H);
	req_set(&rq, id, point_of(RENDER_W, 2), (const char *)enc);
	font_draw(&s, &rq);
	h_eq_i64("BORD-glyphe a xoff negatif dont le bord gauche entre dans la "
		"surface", g_fake.calls, visible_ink(g));
	h_eq_i64("BORD-aucun appel hors surface", g_fake.outside, 0);
	free(s.px);
}

static void	edge_negative_offset_at_right_border(void)
{
	int				id;
	int32_t			k;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id);
		k = 0;
		while (k < f->nglyphs)
		{
			if (f->glyphs[k].xoff < 0 && f->glyphs[k].cp != 0xFFFD)
				check_glyph((t_fontid)id, &f->glyphs[k]);
			k++;
		}
		id++;
	}
}

int	main(void)
{
	h_begin("a16/draw_edge");
	h_run("xoff negatif au bord droit", edge_negative_offset_at_right_border);
	return (h_end());
}
