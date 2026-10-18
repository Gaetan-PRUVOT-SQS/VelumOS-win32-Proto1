#include "harness.h"
#include "velum/font.h"
#include "e15.h"

static void	glyphes_de_la_console(void)
{
	static const uint32_t	cp[] = {0xe9, 0xb7, 0xc9, 0xe8, 0xe0, 0xe7};
	const t_font			*f;
	size_t					i;

	f = font_get(FONT_JBM_14);
	h_true(f != NULL, "police JetBrains Mono 14");
	i = 0;
	while (i < sizeof(cp) / sizeof(cp[0]))
	{
		h_true(font_glyph(f, cp[i]) != NULL, "glyphe present");
		i++;
	}
}

static void	cellule_fixe(void)
{
	const t_font	*f;
	const t_glyph	*m;
	const t_glyph	*i;

	f = font_get(FONT_JBM_14);
	m = font_glyph(f, 'M');
	i = font_glyph(f, 'i');
	h_true(m && i, "glyphes M et i");
	h_eq_u64("chasse fixe", m->advance, i->advance);
}

int	main(void)
{
	h_begin("e15_glyphes");
	h_run("glyphes_de_la_console", glyphes_de_la_console);
	h_run("cellule_fixe", cellule_fixe);
	return (h_end());
}
