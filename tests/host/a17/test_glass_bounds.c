#include <stdlib.h>
#include "harness.h"
#include "lt_gal.h"

#define BACK 0xff3366ccu

static t_color	px(const t_lt *t, int32_t x, int32_t y)
{
	return (t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]);
}

static uint32_t	*scratch_for(t_rect r)
{
	size_t		n;
	uint32_t	*buf;

	n = luna_glass_need(r);
	buf = malloc(n * sizeof(uint32_t));
	if (buf != NULL)
		luna_glass_scratch(buf, n);
	return (buf);
}

static void	glass_clip(void)
{
	t_lt		t;
	uint32_t	*buf;
	t_color		inside;

	buf = scratch_for(lp_rect(20, 20, 300, 80));
	h_true(buf != NULL && lt_open(&t, 400, 140) == 0, "allocation");
	lt_clear(&t, BACK);
	gfx_set_clip(&t.s, lp_rect(100, 0, 100, 140));
	luna_glass(&t.s, lp_rect(20, 20, 300, 80), LUNA_GLASS_PANEL);
	inside = px(&t, 150, 70);
	h_true(inside != BACK, "pixel dans le clip peint");
	h_eq_u64("pixel hors clip, a gauche", px(&t, 60, 70), BACK);
	h_eq_u64("pixel hors clip, a droite", px(&t, 250, 70), BACK);
	h_true(lt_guard_ok(&t), "gardes intactes");
	luna_glass_scratch(NULL, 0);
	free(buf);
	lt_close(&t);
}

static void	glass_off_surface(void)
{
	t_lt		t;
	uint32_t	*buf;

	buf = scratch_for(lp_rect(-60, -40, 300, 200));
	h_true(buf != NULL && lt_open(&t, 200, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(-60, -40, 300, 200), LUNA_GLASS_WINDOW);
	luna_glass(&t.s, lp_rect(150, 90, 300, 200), LUNA_GLASS_CHIP);
	h_true(lt_guard_ok(&t), "aucune ecriture hors de la surface");
	h_true(px(&t, 100, 60) != BACK, "partie visible peinte");
	luna_glass_scratch(NULL, 0);
	free(buf);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_bornes");
	h_run("clip respecte", glass_clip);
	h_run("rectangle hors de la surface", glass_off_surface);
	return (h_end());
}
