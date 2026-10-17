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

static t_color	expected_dock(t_color back)
{
	t_color	c;

	c = lp_saturate(back, VT_SAT_LIGHT_X10);
	return (lp_mix(c, VT_GLASS_DOCK | 0xff000000, VT_GLASS_DOCK >> 24)
		| 0xff000000);
}

static void	glass_uniform(void)
{
	t_lt	t;

	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, DOCK_W, DOCK_H), LUNA_GLASS_DOCK);
	h_eq_u64("centre sous le reflet", px(&t, 220, 60), expected_dock(BACK));
	h_eq_u64("hors du verre, a gauche", px(&t, 10, 40), BACK);
	h_eq_u64("hors du verre, en bas", px(&t, 220, 90), BACK);
	h_true(lt_guard_ok(&t), "gardes intactes");
	lt_close(&t);
}

static void	glass_uniform_run(void)
{
	size_t		n;
	uint32_t	*buf;

	n = luna_glass_need(lp_rect(20, 20, DOCK_W, DOCK_H));
	buf = malloc(n * sizeof(uint32_t));
	h_true(buf != NULL, "allocation du tampon de travail");
	if (buf == NULL)
		return ;
	luna_glass_scratch(buf, n);
	glass_uniform();
	luna_glass_scratch(NULL, 0);
	free(buf);
}

int	main(void)
{
	h_begin("a17/glass");
	h_run("fond uniforme (oracle)", glass_uniform_run);
	return (h_end());
}
