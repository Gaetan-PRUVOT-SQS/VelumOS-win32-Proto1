#include <stdio.h>
#include "harness.h"
#include "lt_bbox.h"
#include "lt_contrast.h"

#define TB_W 1280
#define TB_H 800
#define TB_SCR 1048576
#define TB_PCT 50

static uint32_t	g_scr[TB_SCR];
static t_lt		g_t;
static t_lt		g_b;

static double	strip_worst(const t_surface *s, t_rect z, t_color c)
{
	t_rect	left;
	t_rect	right;
	double	a;
	double	b;

	left = lp_rect(z.x + 8, z.y + 30, 6, z.h - 60);
	right = lp_rect(z.x + z.w - 14, z.y + 30, 6, z.h - 60);
	a = lk_worst(s, left, c);
	b = lk_worst(s, right, c);
	if (b < a)
		return (b);
	return (a);
}

static size_t	count_color(const t_surface *s, t_rect z, t_color c)
{
	size_t	n;
	t_point	p;

	n = 0;
	p.y = z.y;
	while (p.y < z.y + z.h)
	{
		p.x = z.x;
		while (p.x < z.x + z.w)
		{
			if (gfx_get(s, p) == c)
				n++;
			p.x++;
		}
		p.y++;
	}
	return (n);
}

static void	boot_case(const char *tag, bool dark, bool reduced)
{
	t_rect	z;
	char	name[96];
	t_cpal	pal;

	luna_theme_set(LUNA_THEME_LIGHT);
	if (dark)
		luna_theme_set(LUNA_THEME_DARK);
	luna_transparency_set(reduced);
	luna_glass_scratch(g_scr, TB_SCR);
	pal = lp_pal();
	luna_logon_bg(&g_b.s, lp_rect(0, 0, TB_W, TB_H));
	lk_begin(&g_t, LC_BLACK);
	luna_boot_draw(&g_t.s, TB_PCT, 0);
	z = ltb_bbox(&g_t.s, &g_b.s, 0);
	snprintf(name, sizeof(name), "texte de demarrage, %s", tag);
	lk_check(name, lk_spied(&g_t.s));
	snprintf(name, sizeof(name), "marque (encre) sur la carte, %s", tag);
	h_true(strip_worst(&g_t.s, z, pal->ink) >= 4.5, name);
	snprintf(name, sizeof(name), "remplissage accent sur la carte, %s", tag);
	h_true(strip_worst(&g_t.s, z, pal->accent) >= 3.0, name);
	snprintf(name, sizeof(name), "remplissage plein couleur accent, %s", tag);
	h_true(count_color(&g_t.s, z, pal->accent)
		>= (size_t)(280 * TB_PCT / 100 * 4), name);
}

static void	boot_matrix(void)
{
	boot_case("clair, verre", false, false);
	boot_case("clair, transparence reduite", false, true);
	boot_case("sombre, verre", true, false);
	boot_case("sombre, transparence reduite", true, true);
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
	luna_glass_scratch(NULL, 0);
}

int	main(void)
{
	if (lt_open(&g_t, TB_W, TB_H) != 0 || lt_open(&g_b, TB_W, TB_H) != 0)
		return (1);
	h_begin("a17/contrast_boot");
	h_run("P25 ecran de demarrage : texte, marque et barre, 2 themes x 2 modes",
		boot_matrix);
	lt_close(&g_b);
	lt_close(&g_t);
	return (h_end());
}
