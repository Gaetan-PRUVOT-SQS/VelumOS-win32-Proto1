#include "font_int.h"
#include "velum/err.h"

static int	selftest_tables(void)
{
	int				id;
	int32_t			k;
	const t_font	*f;

	id = 0;
	while (id < FONT_IDS)
	{
		f = font_get((t_fontid)id++);
		if (!f || f->nglyphs <= 0 || f->height != f->ascent + f->descent)
			return (E_INVAL);
		k = 1;
		while (k < f->nglyphs)
		{
			if (f->glyphs[k - 1].cp >= f->glyphs[k].cp)
				return (E_INVAL);
			k++;
		}
		if (font_glyph(f, '0')->cp != '0' || font_glyph(f, 0x1F)->cp != 0xFFFD)
			return (E_INVAL);
	}
	if (font_get(FONT_IDS))
		return (E_INVAL);
	return (E_OK);
}

static int	selftest_utf8(void)
{
	const char	*s;
	const char	*end;

	s = "\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\xC0\x80\xE2\x82";
	end = s + 13;
	if (font_utf8_next(&s, end) != 0xE9 || font_utf8_next(&s, end) != 0x20AC)
		return (E_INVAL);
	if (font_utf8_next(&s, end) != 0x1F600 || s != end - 4)
		return (E_INVAL);
	if (font_utf8_next(&s, end) != 0xFFFD || font_utf8_next(&s, end) != 0xFFFD)
		return (E_INVAL);
	if (font_utf8_next(&s, end) != 0xFFFD || s != end)
		return (E_INVAL);
	if (font_utf8_next(&s, end) != 0 || s != end)
		return (E_INVAL);
	return (E_OK);
}

static int	selftest_metrics(void)
{
	const t_font	*mono;

	mono = font_get(FONT_MONO);
	if (font_text_width(mono, "D\xC3\xA9marrer", -1) != 64)
		return (E_INVAL);
	if (font_fit(mono, "D\xC3\xA9marrer", 63) != 8)
		return (E_INVAL);
	if (font_fit(mono, "D\xC3\xA9marrer", 24) != 4)
		return (E_INVAL);
	if (font_text_width(mono, "ab", 1) != 8 || font_text_width(NULL, "a", 1))
		return (E_INVAL);
	return (E_OK);
}

int	font_selftest(void)
{
	int	rc;

	rc = selftest_tables();
	if (rc == E_OK)
		rc = selftest_utf8();
	if (rc == E_OK)
		rc = selftest_metrics();
	return (rc);
}
