#include "a16_test.h"

static const uint8_t	g_bare_bits[] = {0x80, 0x80, 0x80};

static const t_glyph	g_bare_glyphs[] = {
{0x41, 1, 3, 0, 0, 5, 0},
{0x42, 0, 3, 0, 0, 4, 0},
{0x43, 3, 0, 0, 0, 4, 0},
};

static const t_font		g_bare = {3, 0, 3, 3, g_bare_glyphs,
	g_bare_bits, 3};

const t_font	*bare_font(void)
{
	return (&g_bare);
}
