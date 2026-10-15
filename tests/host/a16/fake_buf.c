#include <stdlib.h>
#include <string.h>
#include "a16_test.h"

uint8_t	*exact_copy(const void *src, size_t len)
{
	uint8_t	*copy;
	size_t	size;

	size = len;
	if (size == 0)
		size = 1;
	copy = malloc(size);
	if (!copy)
		abort();
	memcpy(copy, src, len);
	return (copy);
}

uint32_t	decode_poisoned(const uint8_t *seq, size_t len, size_t *used)
{
	uint8_t		buf[16];
	const char	*cur;
	uint32_t	cp;

	memset(buf, 0x80, sizeof(buf));
	memcpy(buf, seq, len);
	cur = (const char *)buf;
	cp = font_utf8_next(&cur, (const char *)buf + len);
	*used = (size_t)(cur - (const char *)buf);
	return (cp);
}

int32_t	count_replacements(const void *seq, size_t len)
{
	uint8_t		*heap;
	const char	*cur;
	const char	*end;
	int32_t		count;

	heap = exact_copy(seq, len);
	cur = (const char *)heap;
	end = cur + len;
	count = 0;
	while (cur < end && count >= 0)
	{
		if (font_utf8_next(&cur, end) == UTF8_REPLACEMENT)
			count++;
		else
			count = -1;
	}
	free(heap);
	return (count);
}

size_t	decode_n(const void *seq, size_t len, uint32_t *out, size_t cap)
{
	uint8_t		*heap;
	const char	*cur;
	const char	*end;
	size_t		count;

	heap = exact_copy(seq, len);
	cur = (const char *)heap;
	end = cur + len;
	count = 0;
	while (cur < end && count < cap)
		out[count++] = font_utf8_next(&cur, end);
	free(heap);
	return (count);
}

int	glyph_pixel(const t_glyph *g, int32_t col, int32_t row)
{
	const uint8_t	*rows;
	const t_font	*f;
	size_t			stride;
	int				id;

	id = 0;
	f = font_get(FONT_UI);
	while (f && !(g >= f->glyphs && g < f->glyphs + f->nglyphs))
		f = font_get((t_fontid)++id);
	if (!font_glyph_rows(f, g, &rows))
		abort();
	stride = (size_t)(g->w + 7) / 8;
	return ((rows[(size_t)row * stride + (size_t)col / 8]
			>> (7 - col % 8)) & 1);
}
