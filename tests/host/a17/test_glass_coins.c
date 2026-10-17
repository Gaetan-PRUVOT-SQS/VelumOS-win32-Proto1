#include <stdlib.h>
#include "velum/theme.h"
#include "harness.h"
#include "lt_gal.h"

#define BACK 0xffc86432u
#define DOCK_W 400
#define DOCK_H 56

static t_color	px(const t_lt *t, int32_t x, int32_t y)
{
	return (t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]);
}

static void	glass_corners(void)
{
	t_lt	t;

	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, DOCK_W, DOCK_H), LUNA_GLASS_DOCK);
	h_eq_u64("coin haut gauche intact", px(&t, 20, 20), BACK);
	h_eq_u64("coin bas droit intact", px(&t, 20 + DOCK_W - 1,
			20 + DOCK_H - 1), BACK);
	h_true(px(&t, 20 + 22, 20 + 28) != BACK, "milieu du bord gauche peint");
	lt_close(&t);
}

static void	glass_corners_run(void)
{
	size_t		n;
	uint32_t	*buf;

	n = luna_glass_need(lp_rect(20, 20, DOCK_W, DOCK_H));
	buf = malloc(n * sizeof(uint32_t));
	h_true(buf != NULL, "allocation du tampon de travail");
	if (buf == NULL)
		return ;
	luna_glass_scratch(buf, n);
	glass_corners();
	luna_glass_scratch(NULL, 0);
	free(buf);
}

int	main(void)
{
	h_begin("a17/glass_coins");
	h_run("coins arrondis", glass_corners_run);
	return (h_end());
}
