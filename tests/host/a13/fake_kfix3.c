#include "kfix.h"

uint32_t	kfix_spaces(void)
{
	uint32_t	i;
	uint32_t	n;

	i = 0;
	n = 0;
	while (i < g_ffont.glyphs && i < FAKE_LOG)
	{
		if (kfix_glyph(i)->cp == ' ')
			n++;
		i++;
	}
	return (n);
}

int	kfix_try(const t_surface *s)
{
	t_kcon	k;

	return (kcon_setup(&k, s, font_get(FONT_MONO)));
}
