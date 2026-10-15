#include "fakes.h"

t_ffont	g_ffont;

void	fake_font_reset(void)
{
	memset(&g_ffont, 0, sizeof(g_ffont));
	g_ffont.font.ascent = 12;
	g_ffont.font.descent = 4;
	g_ffont.font.height = 16;
	g_ffont.cell_w = 8;
}

const t_font	*font_get(t_fontid id)
{
	if (g_ffont.null_font || id != FONT_MONO)
		return (NULL);
	return (&g_ffont.font);
}

int32_t	font_text_width(const t_font *f, const char *text, int32_t len)
{
	const char	*p;
	const char	*q;
	const char	*end;
	int32_t		n;

	(void)f;
	p = text;
	end = text + len;
	n = 0;
	while (p < end)
	{
		q = p;
		font_utf8_next(&q, end);
		if (q <= p)
			q = p + 1;
		p = q;
		n++;
	}
	return (n * g_ffont.cell_w);
}
