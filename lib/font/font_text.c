#include "font_int.h"

const char	*font_text_end(const char *text, int32_t len)
{
	const char	*end;

	end = text;
	while ((len < 0 || end - text < len) && *end)
		end++;
	return (end);
}

int32_t	font_text_width(const t_font *f, const char *text, int32_t len)
{
	const char		*end;
	const t_glyph	*g;
	int64_t			sum;

	if (!f || !text)
		return (0);
	end = font_text_end(text, len);
	sum = 0;
	while (text < end)
	{
		g = font_glyph(f, font_utf8_next(&text, end));
		if (g)
			sum += g->advance;
	}
	if (sum > INT32_MAX)
		return (INT32_MAX);
	return ((int32_t)sum);
}

static int32_t	byte_count(const char *from, const char *to)
{
	if (to - from > INT32_MAX)
		return (INT32_MAX);
	return ((int32_t)(to - from));
}

int32_t	font_fit(const t_font *f, const char *text, int32_t max_w)
{
	const char		*end;
	const char		*cur;
	const char		*next;
	const t_glyph	*g;
	int64_t			left;

	if (!f || !text || max_w < 0)
		return (0);
	end = font_text_end(text, -1);
	cur = text;
	left = max_w;
	while (cur < end)
	{
		next = cur;
		g = font_glyph(f, font_utf8_next(&next, end));
		if (g && g->advance > left)
			break ;
		if (g)
			left -= g->advance;
		cur = next;
	}
	return (byte_count(text, cur));
}
