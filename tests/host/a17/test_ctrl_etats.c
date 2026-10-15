#include "harness.h"
#include "lt.h"
#include "luna_ctrl.h"
#include "velum/theme.h"

static t_lt	g_t;

static t_color	over(t_color bg, uint32_t percent)
{
	t_color		out;
	int32_t		shift;
	uint32_t	b;
	uint32_t	alpha;

	alpha = (percent * 255 + 50) / 100;
	out = 0xff000000U;
	shift = 0;
	while (shift < 24)
	{
		b = (bg >> shift) & 255;
		out |= (((b * (255 - alpha) + 255 * alpha + 127) / 255) & 255)
			<< shift;
		shift += 8;
	}
	return (out);
}

static void	etat_secondaire(t_lunastate st, t_color want, const char *msg)
{
	t_lunabtn	b;
	t_color		got;
	int32_t		shift;
	int32_t		d;

	b.r = lp_rect(10, 10, 80, 28);
	b.state = st;
	b.is_default = false;
	b.focus = false;
	luna_button(&g_t.s, &b);
	got = gfx_get(&g_t.s, lp_pt(50, 24));
	shift = 0;
	while (shift < 24)
	{
		d = (int32_t)(got >> shift & 255) - (int32_t)(want >> shift & 255);
		h_true(d >= -2 && d <= 2, msg);
		shift += 8;
	}
}

static void	table_fonds_du_secondaire(void)
{
	t_color	base;

	h_true(lt_open(&g_t, 100, 48) == 0, "allocation");
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_clear(&g_t, VT_WALL_BASE);
	base = over(VT_WALL_BASE, 55);
	etat_secondaire(LS_NORMAL, base, "clair normal : blanc 55 %");
	lt_clear(&g_t, VT_WALL_BASE);
	etat_secondaire(LS_HOT, over(base, 40), "clair survol : blanc 40 %");
	lt_clear(&g_t, VT_WALL_BASE);
	etat_secondaire(LS_PRESSED, over(base, 70), "clair appui : blanc 70 %");
	luna_theme_set(LUNA_THEME_DARK);
	lt_clear(&g_t, VT_WALL_BASE_D);
	base = over(VT_WALL_BASE_D, 8);
	etat_secondaire(LS_HOT, over(base, 8), "sombre survol : blanc 8 %");
	lt_clear(&g_t, VT_WALL_BASE_D);
	etat_secondaire(LS_PRESSED, over(base, 14), "sombre appui : blanc 14 %");
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&g_t);
}

static void	champ_en_pilule(void)
{
	h_true(lt_open(&g_t, 140, 70) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	luna_edit_frame(&g_t.s, lp_rect(10, 10, 120, 48), true);
	h_eq_u64("pilule de 48 : coin haut gauche vide",
		gfx_get(&g_t.s, lp_pt(14, 14)), LC_WHITE);
	h_true(gfx_get(&g_t.s, lp_pt(10, 34)) != LC_WHITE, "bord gauche trace");
	h_true(gfx_get(&g_t.s, lp_pt(70, 10)) != LC_WHITE, "bord haut trace");
	lt_close(&g_t);
}

int	main(void)
{
	h_begin("a17/ctrl_etats");
	h_run("table de decision : fonds du secondaire (README 7)",
		table_fonds_du_secondaire);
	h_run("partition : champ de saisie en pilule", champ_en_pilule);
	return (h_end());
}
