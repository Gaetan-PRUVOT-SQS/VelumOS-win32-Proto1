#include <stdint.h>
#include "harness.h"
#include "lt_fuzz.h"

#define BACK_A 0xff3366ccu
#define BACK_B 0xffcc9933u
#define SCR 4096
#define CX 50
#define CY 25

static uint32_t	g_scr[SCR];

static t_color	center(t_lt *t, t_color back, int32_t kind, int32_t mode)
{
	lt_clear(t, back);
	luna_glass_scratch(g_scr, SCR);
	if (mode == 0)
		luna_glass_scratch(NULL, 0);
	luna_transparency_set(mode == 1);
	luna_glass(&t->s, lp_rect(20, 10, 60, 30), (t_glasskind)kind);
	return (t->s.px[CY * (size_t)t->s.stride + CX]);
}

static void	fallback_kind(t_lt *t, int32_t kind)
{
	h_eq_u64("repli sans tampon : centre independant du fond",
		center(t, BACK_A, kind, 0), center(t, BACK_B, kind, 0));
	h_eq_u64("transparence reduite : centre independant du fond",
		center(t, BACK_A, kind, 1), center(t, BACK_B, kind, 1));
	h_true(lt_guard_ok(t), "gardes intactes");
}

static void	fallback_table(void)
{
	t_lt	t;
	int32_t	k;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	k = -1;
	while (++k < LUNA_GLASS_KINDS)
		fallback_kind(&t, k);
	luna_transparency_set(false);
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

static void	blur_depends(void)
{
	t_lt	t;

	h_true(lt_open(&t, 100, 60) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	h_true(center(&t, BACK_A, LUNA_GLASS_DOCK, 2)
		!= center(&t, BACK_B, LUNA_GLASS_DOCK, 2),
		"temoin : avec flou, le centre depend du fond");
	luna_glass_scratch(NULL, 0);
	lt_close(&t);
}

int	main(void)
{
	h_begin("a17/glass_repli");
	h_run("repli opaque, relation metamorphique sur le fond", fallback_table);
	h_run("temoin de sensibilite du flou", blur_depends);
	return (h_end());
}
