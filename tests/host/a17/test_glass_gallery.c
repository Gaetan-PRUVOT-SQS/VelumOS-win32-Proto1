#include <stdlib.h>
#include "harness.h"
#include "lt_gal.h"

#define SW 1280
#define SH 800

static void	scene(t_lt *t, const char *path)
{
	luna_wallpaper(&t->s, lp_rect(0, 0, SW, SH));
	luna_glass(&t->s, lp_rect(12, SH - 68, SW - 24, 56), LUNA_GLASS_DOCK);
	luna_glass(&t->s, lp_rect(12, 431, 540, 289), LUNA_GLASS_LAUNCHER);
	luna_glass(&t->s, lp_rect(330, 160, 480, 235), LUNA_GLASS_WINDOW);
	luna_glass(&t->s, lp_rect(700, 440, 380, 230), LUNA_GLASS_CARD);
	luna_glass(&t->s, lp_rect(760, 700, 260, 44), LUNA_GLASS_CHIP);
	luna_glass(&t->s, lp_rect(600, 40, 420, 90), LUNA_GLASS_CONSOLE);
	h_true(lt_png(path, &t->s, 1) == 0, "ecriture png");
}

static void	gallery(void)
{
	t_lt		t;
	uint32_t	*buf;
	size_t		n;

	n = luna_glass_need(lp_rect(0, 0, SW, SH));
	buf = malloc(n * sizeof(uint32_t));
	h_true(buf != NULL && lt_open(&t, SW, SH) == 0, "allocation");
	luna_glass_scratch(buf, n);
	lt_out_dir();
	luna_theme_set(LUNA_THEME_LIGHT);
	scene(&t, "build/a17/verre_clair.png");
	luna_theme_set(LUNA_THEME_DARK);
	scene(&t, "build/a17/verre_sombre.png");
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_glass_scratch(NULL, 0);
	free(buf);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_galerie");
	h_run("clair et sombre", gallery);
	return (h_end());
}
