#include <stdint.h>
#include "harness.h"
#include "a20_srgb.h"
#include "velum/theme.h"

static void	contrast_formule_de_reference(void)
{
	h_eq_u64("noir sur blanc: 21", ratio_x100(0x000000, 0xffffff), 2100);
	h_eq_u64("meme couleur: 1", ratio_x100(0x245edc, 0x245edc), 100);
	h_eq_u64("symetrique", ratio_x100(0xffffff, 0x245edc),
		ratio_x100(0x245edc, 0xffffff));
	h_true(ratio_x100(0x777777, 0xffffff) < 450, "#777 sur blanc sous AA");
	h_true(ratio_x100(0x767676, 0xffffff) >= 450, "#767676 sur blanc: AA");
	h_true(ratio_x100(0xffffff, 0x245edc) >= 560, "blanc sur #245EDC: 5,6");
	h_true(ratio_x100(0xffffff, 0x245edc) <= 570, "blanc sur #245EDC: 5,7");
}

static uint32_t	sature(uint32_t rgb, int32_t sat10)
{
	int32_t		lum;
	int32_t		c;
	int32_t		shift;
	uint32_t	out;

	lum = (int32_t)((299 * ((rgb >> 16) & 255) + 587 * ((rgb >> 8) & 255)
				+ 114 * (rgb & 255)) / 1000);
	out = 0;
	shift = 16;
	while (shift >= 0)
	{
		c = lum + (((int32_t)((rgb >> shift) & 255) - lum) * sat10) / 10;
		if (c < 0)
			c = 0;
		if (c > 255)
			c = 255;
		out |= (uint32_t)c << shift;
		shift -= 8;
	}
	return (out);
}

static void	encre_sur_le_dock(const uint32_t *walls, const uint32_t *tints,
		int32_t sat10, uint32_t ink)
{
	uint32_t	dock;
	uint32_t	plate;
	int32_t		i;

	i = 0;
	while (i < 5)
	{
		dock = blend_over(tints[0], sature(walls[i], sat10));
		plate = blend_over(tints[1], dock);
		h_true(ratio_x100(ink & 0xffffff, dock) >= 450,
			"encre sur le verre du dock, pire fond de la palette");
		h_true(ratio_x100(ink & 0xffffff, plate) >= 450,
			"encre sur la plaque de la tache active");
		i++;
	}
}

static void	contrast_dock_clair_et_sombre(void)
{
	static const uint32_t	light[5] = {VT_WALL_BASE, VT_WALL_TEAL,
		VT_WALL_VIOLET, VT_WALL_AMBER, VT_WALL_ROSE};
	static const uint32_t	dark[5] = {VT_WALL_BASE_D, VT_WALL_TEAL_D,
		VT_WALL_VIOLET_D, VT_WALL_AMBER_D, VT_WALL_ROSE_D};
	static const uint32_t	tint_l[2] = {VT_GLASS_DOCK, 3019898879U};
	static const uint32_t	tint_d[2] = {VT_GLASS_DOCK_D, 0x29ffffff};

	encre_sur_le_dock(light, tint_l, 18, VT_INK);
	encre_sur_le_dock(dark, tint_d, 16, VT_INK_D);
}

int	main(void)
{
	h_begin("a20/contrast");
	h_run("contraste: formule de reference", contrast_formule_de_reference);
	h_run("contraste: dock clair et sombre", contrast_dock_clair_et_sombre);
	return (h_end());
}
