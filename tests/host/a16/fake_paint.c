#include "a16_test.h"

static void	model_glyph(uint8_t *ink, const t_glyph *g, t_point at)
{
	int32_t	row;
	int32_t	col;
	int32_t	x;
	int32_t	y;

	row = 0;
	while (row < g->h)
	{
		col = 0;
		while (col < g->w)
		{
			x = at.x + col;
			y = at.y + row;
			if (glyph_pixel(g, col, row) && x >= 0 && x < RENDER_W && y >= 0
				&& y < RENDER_H)
				ink[y * RENDER_W + x] = 1;
			col++;
		}
		row++;
	}
}

void	ink_of(const t_font *f, const char *text, t_point at, uint8_t *ink)
{
	const t_glyph	*g;
	const char		*end;
	t_point			cell;

	end = text + strlen(text);
	while (text < end)
	{
		g = font_glyph(f, font_utf8_next(&text, end));
		cell = point_of(at.x + g->xoff, at.y + g->yoff);
		if (g->w)
			model_glyph(ink, g, cell);
		at.x += g->advance;
	}
}
