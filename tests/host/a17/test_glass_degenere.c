#include "harness.h"
#include "lt_gal.h"

#define BACK 0xff3366ccu

static t_color	px(const t_lt *t, int32_t x, int32_t y)
{
	return (t->s.px[(size_t)y * (size_t)t->s.stride + (size_t)x]);
}

static void	glass_degenerate(void)
{
	t_lt	t;

	h_true(lt_open(&t, 100, 100) == 0, "allocation");
	lt_clear(&t, BACK);
	luna_glass(&t.s, lp_rect(10, 10, 0, 50), LUNA_GLASS_DOCK);
	luna_glass(&t.s, lp_rect(10, 10, 50, -3), LUNA_GLASS_DOCK);
	luna_glass(&t.s, lp_rect(10, 10, 50, 50), (t_glasskind)99);
	luna_glass(NULL, lp_rect(10, 10, 50, 50), LUNA_GLASS_DOCK);
	h_eq_u64("rien n'est peint", px(&t, 30, 30), BACK);
	h_true(lt_guard_ok(&t), "gardes intactes");
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_degenere");
	h_run("entrees degenerees", glass_degenerate);
	return (h_end());
}
