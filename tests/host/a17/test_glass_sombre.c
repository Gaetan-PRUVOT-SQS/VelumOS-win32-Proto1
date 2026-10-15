#include "velum/theme.h"
#include "harness.h"
#include "lt_gal.h"

#define BACK 0xffc86432u

static t_color	px(const t_lt *t, int32_t x, int32_t y)
{
	return (t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]);
}

static void	dark_theme(void)
{
	t_lt	t;

	luna_theme_set(LUNA_THEME_DARK);
	luna_glass_scratch(NULL, 0);
	h_true(lt_open(&t, 480, 120) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(20, 20, 400, 56), LUNA_GLASS_DOCK);
	h_eq_u64("opaque sombre", px(&t, 220, 60), VT_OPAQUE_PANEL_D);
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_sombre");
	h_run("theme sombre", dark_theme);
	return (h_end());
}
