#include "harness.h"
#include "lt.h"
#include "luna_ctrl.h"
#include "velum/theme.h"

static t_lt			g_t;

static const t_rect	g_rects[] = {{-5, -5, 30, 30}, {35, 15, 20, 20},
{0, 0, 0, 0}, {5, 5, -3, 4}, {INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX}};

static void	bornes_et_gardes(void)
{
	t_lrfill	f;
	int32_t		i;

	h_true(lt_open(&g_t, 40, 20) == 0, "allocation");
	lt_clear(&g_t, LC_BLACK);
	i = 0;
	while (i < 5)
	{
		f = lp_rmake(g_rects[i], 8, 0x80ffffff, 0x40000000);
		lp_rfill(&g_t.s, &f);
		lp_rring(&g_t.s, &f, 2);
		lp_rtop(&g_t.s, &f);
		lp_rfocus(&g_t.s, g_rects[i], 8, VT_ACCENT);
		i++;
	}
	lp_rfill(NULL, &f);
	lp_rfill(&g_t.s, NULL);
	lp_rring(&g_t.s, NULL, 1);
	lp_rtop(&g_t.s, NULL);
	h_true(lt_guard_ok(&g_t), "aucune ecriture hors de la surface");
	lt_close(&g_t);
}

static void	taille_cases(void)
{
	h_eq_i64("taille de la case", lm_detail()->check_size, 13);
	h_true(lt_open(&g_t, 60, 40) == 0, "allocation");
	lt_clear(&g_t, LC_BLACK);
	luna_checkbox(&g_t.s, lp_pt(20, 10), LS_NORMAL, true);
	luna_radio(&g_t.s, lp_pt(40, 10), LS_NORMAL, true);
	h_true(gfx_get(&g_t.s, lp_pt(26, 16)) != LC_BLACK, "case dessinee");
	h_true(gfx_get(&g_t.s, lp_pt(46, 16)) != LC_BLACK, "radio dessine");
	h_eq_u64("hors case, a droite", gfx_get(&g_t.s, lp_pt(33, 16)),
		0xff000000U);
	h_eq_u64("hors case, a gauche", gfx_get(&g_t.s, lp_pt(19, 16)),
		0xff000000U);
	h_eq_u64("hors radio, a droite", gfx_get(&g_t.s, lp_pt(53, 16)),
		0xff000000U);
	lt_close(&g_t);
}

static void	marge_de_focus(void)
{
	t_lunabtn	b;

	h_true(lt_open(&g_t, 100, 60) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	b.r = lp_rect(20, 14, 60, 32);
	b.state = LS_NORMAL;
	b.is_default = false;
	b.focus = true;
	luna_button(&g_t.s, &b);
	h_eq_u64("anneau externe a gauche", gfx_get(&g_t.s, lp_pt(16, 30)),
		VT_ACCENT);
	h_eq_u64("anneau interne a gauche", gfx_get(&g_t.s, lp_pt(17, 30)),
		VT_ACCENT);
	h_eq_u64("jeu de 2 px a gauche", gfx_get(&g_t.s, lp_pt(18, 30)), LC_WHITE);
	h_eq_u64("anneau a droite", gfx_get(&g_t.s, lp_pt(82, 30)), VT_ACCENT);
	h_eq_u64("anneau en haut", gfx_get(&g_t.s, lp_pt(50, 10)), VT_ACCENT);
	h_eq_u64("anneau en bas", gfx_get(&g_t.s, lp_pt(50, 49)), VT_ACCENT);
	lt_clear(&g_t, LC_WHITE);
	b.focus = false;
	luna_button(&g_t.s, &b);
	h_eq_u64("sans focus, rien dans la marge", gfx_get(&g_t.s, lp_pt(16, 30)),
		LC_WHITE);
	lt_close(&g_t);
}

int	main(void)
{
	luna_theme_set(LUNA_THEME_LIGHT);
	h_begin("a17/ctrl_bornes");
	h_run("valeurs limites : formes degenerees et hors surface",
		bornes_et_gardes);
	h_run("valeurs limites : case et radio de 13 px", taille_cases);
	h_run("table de decision : anneau de focus a 4 px", marge_de_focus);
	return (h_end());
}
