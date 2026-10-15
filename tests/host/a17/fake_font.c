#include "fake_font.h"

static const t_fkp	g_fkp[FONT_IDS] = {{6, 0, 0, 0, 11, 9, 2},
{7, 1, 0, 0, 11, 9, 2}, {7, 1, 1, 0, 13, 10, 3}, {8, 0, 2, 1, 16, 12, 4}};
static t_font		g_fonts[FONT_IDS];

const t_fkp	*fk_params(const t_font *f)
{
	return (&g_fkp[f - g_fonts]);
}

const t_font	*font_get(t_fontid id)
{
	if ((int)id < 0 || id >= FONT_IDS)
		return (NULL);
	g_fonts[id].ascent = g_fkp[id].ascent;
	g_fonts[id].descent = g_fkp[id].descent;
	g_fonts[id].height = g_fkp[id].height;
	return (&g_fonts[id]);
}

int32_t	font_text_width(const t_font *f, const char *text, int32_t len)
{
	const char	*end;
	int32_t		n;

	end = text + len;
	n = 0;
	while (text < end)
	{
		fk_utf8(&text, end);
		n++;
	}
	return (n * fk_params(f)->adv);
}

int32_t	font_fit(const t_font *f, const char *text, int32_t max_w)
{
	const char	*p;
	const char	*end;
	int32_t		w;

	end = text + fk_len(text);
	p = text;
	w = 0;
	while (p < end && w + fk_params(f)->adv <= max_w)
	{
		fk_utf8(&p, end);
		w += fk_params(f)->adv;
	}
	return ((int32_t)(p - text));
}
