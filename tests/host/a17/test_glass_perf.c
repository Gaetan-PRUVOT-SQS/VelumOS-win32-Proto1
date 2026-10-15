#include <stdint.h>
#include <time.h>
#include "harness.h"
#include "lt_fuzz.h"

#define BACK 0xff3366ccu
#define BIG 20000
#define SHIFT 40
#define NKIND 4
#define BUDGET_MS 2000

static const int32_t	g_kind[NKIND] = {LUNA_GLASS_CHIP, LUNA_GLASS_DOCK,
	LUNA_GLASS_WINDOW, LUNA_GLASS_CONSOLE};

static void	render(t_lt *t, t_rect r, int32_t kind)
{
	lt_clear(t, BACK);
	luna_glass_scratch(NULL, 0);
	luna_transparency_set(true);
	luna_glass(&t->s, r, (t_glasskind)kind);
}

static void	crop_kind(t_lt *small, t_lt *big, int32_t kind)
{
	t_point	p;
	bool	same;

	render(small, lp_rect(-SHIFT / 2, -SHIFT / 2, 100, 100), kind);
	render(big, lp_rect(SHIFT / 2, SHIFT / 2, 100, 100), kind);
	same = true;
	p.y = -1;
	while (++p.y < small->h)
	{
		p.x = -1;
		while (++p.x < small->w)
			same = same && small->s.px[(size_t)p.y * (size_t)small->s.stride
				+ (size_t)p.x] == big->s.px[(size_t)(p.y + SHIFT)
				* (size_t)big->s.stride + (size_t)(p.x + SHIFT)];
	}
	h_true(same, "coins partiellement visibles : recadrage identique");
	h_true(lt_guard_ok(small) && lt_guard_ok(big), "gardes intactes");
}

static void	crop_equal(void)
{
	t_lt	small;
	t_lt	big;
	int32_t	i;

	h_true(lt_open(&small, 64, 64) == 0, "allocation");
	h_true(lt_open(&big, 160, 160) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	i = -1;
	while (++i < NKIND)
		crop_kind(&small, &big, g_kind[i]);
	luna_transparency_set(false);
	lt_close(&small);
	lt_close(&big);
}

static void	huge_rect_bounded(void)
{
	t_lt	t;
	clock_t	t0;
	int32_t	i;

	h_true(lt_open(&t, 64, 64) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	t0 = clock();
	i = -1;
	while (++i < NKIND)
	{
		render(&t, lp_rect(0, 0, BIG, BIG), g_kind[i]);
		render(&t, lp_rect(-BIG + 30, -BIG + 30, BIG, BIG), g_kind[i]);
	}
	h_true((clock() - t0) * 1000 / CLOCKS_PER_SEC < BUDGET_MS,
		"rectangle de 20000 px : temps CPU borne");
	h_true(lt_guard_ok(&t), "gardes intactes");
	luna_transparency_set(false);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_perf");
	h_run("coins visibles en partie, recadrage egal", crop_equal);
	h_run("grand rectangle, cout borne par la zone visible",
		huge_rect_bounded);
	return (h_end());
}
