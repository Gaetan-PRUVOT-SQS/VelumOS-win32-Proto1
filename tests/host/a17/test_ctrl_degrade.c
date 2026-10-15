#include "harness.h"
#include "lt_contrast.h"
#include "velum/theme.h"

static t_lt	g_t;

static void	draw_principal(void)
{
	t_lunabtn	b;

	luna_theme_set(LUNA_THEME_LIGHT);
	h_true(lt_open(&g_t, 140, 52) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	b.r = lp_rect(10, 10, 120, 32);
	b.state = LS_NORMAL;
	b.is_default = true;
	b.focus = false;
	luna_button(&g_t.s, &b);
}

static double	ratio_at(int32_t y)
{
	return (lk_ratio(LC_WHITE, gfx_get(&g_t.s, lp_pt(70, y))));
}

static void	contraste_principal(void)
{
	double	worst;
	int32_t	y;

	draw_principal();
	worst = 21.0;
	y = 21;
	while (y < 30)
	{
		if (ratio_at(y) < worst)
			worst = ratio_at(y);
		y++;
	}
	printf("  blanc sur principal : haut %.2f, pire de la bande %.2f, "
		"bas %.2f\n", ratio_at(11), worst, ratio_at(41));
	h_true(worst >= 3.0, "texte blanc gras sur la bande centrale >= 3,0");
	lt_close(&g_t);
}

static void	degrade_vertical(void)
{
	double	prev;
	int32_t	y;

	draw_principal();
	prev = 0.0;
	y = 11;
	while (y < 42)
	{
		h_true(ratio_at(y) >= prev, "le degrade s'assombrit vers le bas");
		prev = ratio_at(y);
		y++;
	}
	h_eq_u64("derniere ligne : accent", gfx_get(&g_t.s, lp_pt(70, 41)),
		VT_ACCENT);
	lt_close(&g_t);
}

int	main(void)
{
	h_begin("a17/ctrl_degrade");
	h_run("oracle independant : contraste du texte blanc",
		contraste_principal);
	h_run("propriete : degrade monotone et extremites", degrade_vertical);
	return (h_end());
}
