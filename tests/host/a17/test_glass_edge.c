#include <stdlib.h>
#include "velum/theme.h"
#include "harness.h"
#include "lt_gal.h"

#define BACK 0xffc86432u

static t_color	px(const t_lt *t, int32_t x, int32_t y)
{
	return (t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]);
}

static void	fallback_no_scratch(void)
{
	t_lt	t;

	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	luna_glass_scratch(NULL, 0);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, 400, 56), LUNA_GLASS_DOCK);
	h_eq_u64("opaque sans tampon", px(&t, 220, 60), VT_OPAQUE_PANEL);
	h_true(lt_guard_ok(&t), "gardes intactes");
	lt_close(&t);
}

static void	fallback_small_scratch(void)
{
	t_lt		t;
	uint32_t	tiny[16];

	luna_glass_scratch(tiny, 16);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, 400, 56), LUNA_GLASS_DOCK);
	h_eq_u64("opaque, tampon trop petit", px(&t, 220, 60), VT_OPAQUE_PANEL);
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

static void	fallback_reduced(void)
{
	t_lt		t;
	uint32_t	*buf;
	size_t		n;

	n = luna_glass_need(lp_rect(20, 20, 400, 56));
	buf = malloc(n * sizeof(uint32_t));
	h_true(buf != NULL, "allocation du tampon");
	luna_glass_scratch(buf, n);
	luna_transparency_set(true);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, 400, 56), LUNA_GLASS_DOCK);
	h_eq_u64("opaque, transparence reduite", px(&t, 220, 60), VT_OPAQUE_PANEL);
	luna_transparency_set(false);
	luna_glass_scratch(NULL, 0);
	free(buf);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_repli");
	h_run("sans tampon de travail", fallback_no_scratch);
	h_run("tampon trop petit", fallback_small_scratch);
	h_run("transparence reduite", fallback_reduced);
	return (h_end());
}
