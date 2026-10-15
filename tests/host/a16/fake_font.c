#include <stdlib.h>
#include "a16_test.h"

static const t_glyph	g_wide_glyphs[] = {
{0x41, 0, 0, 0, 0, 255, 0},
{0xFFFD, 0, 0, 0, 0, 1, 0},
};

static const t_font		g_wide = {8, 2, 10, 2, g_wide_glyphs, NULL,
	0};

const t_font	*wide_font(void)
{
	return (&g_wide);
}

char	*repeat_char(char c, size_t count)
{
	char	*text;

	text = malloc(count + 1);
	if (!text)
		abort();
	memset(text, c, count);
	text[count] = '\0';
	return (text);
}

t_point	point_of(int32_t x, int32_t y)
{
	t_point	p;

	p.x = x;
	p.y = y;
	return (p);
}

void	req_set(t_textreq *rq, t_fontid id, t_point at, const char *s)
{
	rq->font = font_get(id);
	rq->at = at;
	rq->color = FAKE_INK;
	rq->text = s;
	rq->len = -1;
}

int32_t	glyph_ink(const t_glyph *g)
{
	int32_t	count;
	int32_t	row;
	int32_t	col;

	count = 0;
	row = 0;
	while (row < g->h)
	{
		col = 0;
		while (col < g->w)
			count += glyph_pixel(g, col++, row);
		row++;
	}
	return (count);
}
