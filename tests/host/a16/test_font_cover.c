#include "a16_test.h"

#define SCAN_END 0x2100
#define NNBSP 0x202F

static uint32_t	expected_cp(t_cset set, uint32_t cp)
{
	if (charset_has(set, cp))
		return (cp);
	if (cp == NNBSP)
		return (0x20);
	return (0xFFFD);
}

static void	check_range(const t_font *f, t_cset set)
{
	uint32_t	cp;

	cp = 0;
	while (cp < SCAN_END)
	{
		h_eq_u64("COUV-present dans le jeu, sinon remplacement",
			font_glyph(f, cp)->cp, expected_cp(set, cp));
		cp++;
	}
}

static void	cover_required_characters(void)
{
	int	id;

	id = 0;
	while (id < FONT_IDS)
	{
		check_range(font_get((t_fontid)id), charset_of((t_fontid)id));
		id++;
	}
}

int	main(void)
{
	h_begin("a16/font_cover");
	h_run("couverture exigee par jeu de caracteres", cover_required_characters);
	return (h_end());
}
