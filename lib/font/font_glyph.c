#include "font_int.h"

static const t_glyph	*glyph_direct(const t_font *f, uint32_t cp)
{
	uint32_t	slot;

	if (cp < FONT_ASCII_FIRST || cp > FONT_ASCII_LAST)
		return (NULL);
	slot = cp - FONT_ASCII_FIRST;
	if (slot >= (uint32_t)f->nglyphs || f->glyphs[slot].cp != cp)
		return (NULL);
	return (&f->glyphs[slot]);
}

static const t_glyph	*glyph_search(const t_font *f, uint32_t cp)
{
	int32_t	low;
	int32_t	high;
	int32_t	mid;

	low = 0;
	high = f->nglyphs - 1;
	while (low <= high)
	{
		mid = low + (high - low) / 2;
		if (f->glyphs[mid].cp == cp)
			return (&f->glyphs[mid]);
		if (f->glyphs[mid].cp < cp)
			low = mid + 1;
		else
			high = mid - 1;
	}
	return (NULL);
}

static const t_glyph	*glyph_blank(const t_font *f, uint32_t cp)
{
	if (cp != FONT_NBSP && cp != FONT_NNBSP)
		return (NULL);
	return (glyph_search(f, FONT_SPACE));
}

const t_glyph	*font_glyph(const t_font *f, uint32_t cp)
{
	const t_glyph	*found;

	if (!f || !f->glyphs || f->nglyphs <= 0)
		return (NULL);
	found = glyph_direct(f, cp);
	if (!found)
		found = glyph_search(f, cp);
	if (!found)
		found = glyph_blank(f, cp);
	if (!found)
		found = glyph_search(f, FONT_REPLACEMENT);
	return (found);
}
