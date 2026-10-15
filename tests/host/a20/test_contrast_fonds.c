#include <stdint.h>
#include "harness.h"
#include "a20_srgb.h"
#include "uicolor.h"

static const uint32_t	g_walls[] = {VT_WALL_BASE & 0xffffff,
	VT_WALL_TEAL & 0xffffff, VT_WALL_VIOLET & 0xffffff,
	VT_WALL_AMBER & 0xffffff, VT_WALL_ROSE & 0xffffff, 0xffffff};
static const uint32_t	g_walls_d[] = {VT_WALL_BASE_D & 0xffffff,
	VT_WALL_TEAL_D & 0xffffff, VT_WALL_VIOLET_D & 0xffffff,
	VT_WALL_AMBER_D & 0xffffff, VT_WALL_ROSE_D & 0xffffff};

static int	all_above(uint32_t text, uint32_t plate, const uint32_t *bgs,
	int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		if (ratio_x100(text & 0xffffff, blend_over(plate, bgs[i])) < 450)
			return (0);
		i++;
	}
	return (1);
}

static void	fonds_variables_theme_clair(void)
{
	h_true(all_above(UI_WHITE, UI_LABEL_PLATE, g_walls, 6),
		"plaque sombre d'icone: blanc sur tous les fonds clairs");
	h_true(all_above(UI_WHITE, UI_PROMPT_PLATE, g_walls, 6),
		"invite de connexion: blanc sur tous les fonds clairs");
	h_true(all_above(UI_INK, UI_LABEL_BG, g_walls, 6),
		"etiquette d'icone claire: encre sur tous les fonds clairs");
	h_true(all_above(UI_INK, VT_GLASS_DOCK, g_walls, 6),
		"horloge: encre sur le dock, tous les fonds clairs");
}

static void	fonds_variables_theme_sombre(void)
{
	h_true(all_above(UI_INK_D, UI_LABEL_BG_D, g_walls_d, 5),
		"etiquette d'icone sombre: texte clair sur tous les fonds");
	h_true(all_above(UI_INK_D, VT_GLASS_DOCK_D, g_walls_d, 5),
		"horloge: texte clair sur le dock sombre");
	h_true(all_above(UI_WHITE, UI_PROMPT_PLATE, g_walls_d, 5),
		"invite de connexion: blanc sur tous les fonds sombres");
}

int	main(void)
{
	h_begin("a20/contrast-fonds");
	h_run("contraste: fonds variables clair", fonds_variables_theme_clair);
	h_run("contraste: fonds variables sombre", fonds_variables_theme_sombre);
	return (h_end());
}
