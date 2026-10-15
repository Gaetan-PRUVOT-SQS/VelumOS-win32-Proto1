#include <stdint.h>
#include "harness.h"
#include "velum/luna_launcher.h"
#include "render.h"

static size_t	need_px(int32_t w, int32_t h)
{
	return (2 * (size_t)((w + 3) / 4) * (size_t)((h + 3) / 4));
}

static void	tampon_maximum_dock_et_lanceur(void)
{
	h_eq_u64("1024: lanceur 540x540 domine le dock 1000x56",
		sh_glass_px(1024), need_px(540, 540));
	h_eq_u64("1280: lanceur 540x540", sh_glass_px(1280), need_px(540, 540));
	h_eq_u64("8000: dock pleine largeur domine", sh_glass_px(8000),
		need_px(8000 - 24, 56));
	h_eq_u64("ecran nul: lanceur seul", sh_glass_px(0), need_px(540, 540));
	h_eq_u64("limite basse: 5228 -> lanceur", sh_glass_px(5228),
		need_px(540, 540));
	h_eq_u64("limite haute: 5229 -> dock", sh_glass_px(5229),
		need_px(5229 - 24, 56));
}

static void	tampon_couvre_le_lanceur_reel(void)
{
	size_t	px;

	px = need_px(LUNA_LAUNCHER_WIDTH, LUNA_LAUNCHER_HEIGHT);
	h_true(sh_glass_px(1024) >= px, "1024: lanceur de lib/luna couvert");
	h_true(sh_glass_px(1280) >= px, "1280: lanceur de lib/luna couvert");
	h_true(sh_glass_px(0) >= px, "ecran nul: lanceur de lib/luna couvert");
}

int	main(void)
{
	h_begin("a20/render-dock-buf");
	h_run("tampon de verre: maximum dock et lanceur",
		tampon_maximum_dock_et_lanceur);
	h_run("tampon de verre: lanceur reel de lib/luna",
		tampon_couvre_le_lanceur_reel);
	return (h_end());
}
