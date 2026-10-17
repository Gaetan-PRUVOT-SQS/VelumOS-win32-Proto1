#include "harness.h"
#include "lt.h"
#include "luna_ctrl.h"
#include "velum/theme.h"

static t_lt	g_t;

static void	radio_coche_anneau_4px(void)
{
	int32_t	x;

	luna_theme_set(LUNA_THEME_LIGHT);
	h_true(lt_open(&g_t, 40, 30) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	luna_radio(&g_t.s, lp_pt(10, 8), LS_NORMAL, true);
	x = 10;
	while (x < 14)
	{
		h_eq_u64("anneau accent de 4 px, rangee centrale",
			gfx_get(&g_t.s, lp_pt(x, 14)), VT_ACCENT);
		x++;
	}
	h_true(gfx_get(&g_t.s, lp_pt(14, 14)) != VT_ACCENT, "centre non accent");
	luna_theme_set(LUNA_THEME_DARK);
	lt_clear(&g_t, 0xff0d1120);
	luna_radio(&g_t.s, lp_pt(10, 8), LS_NORMAL, true);
	h_eq_u64("sombre : anneau accent #2BB3B1", gfx_get(&g_t.s, lp_pt(12, 14)),
		0xff2bb3b1U);
	luna_theme_set(LUNA_THEME_LIGHT);
	lt_close(&g_t);
}

static void	radio_decoche_contour_gris(void)
{
	t_color	bord;
	t_color	mi;

	h_true(lt_open(&g_t, 40, 30) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	luna_radio(&g_t.s, lp_pt(10, 8), LS_NORMAL, false);
	h_eq_u64("contour 1,5 px #8A93A8 : colonne 0",
		gfx_get(&g_t.s, lp_pt(10, 14)), 0xff8a93a8U);
	bord = gfx_get(&g_t.s, lp_pt(11, 14));
	mi = 0xff000000U | ((255 + 0x8a) / 2) << 16 | ((255 + 0x93) / 2) << 8
		| ((255 + 0xa8) / 2);
	h_true(bord != LC_WHITE && bord != 0xff8a93a8U, "colonne 1 : demi-teinte");
	h_true(((bord >> 16 & 255) - (mi >> 16 & 255) + 2) <= 4U,
		"colonne 1 : environ 50 % de #8A93A8");
	h_eq_u64("interieur blanc", gfx_get(&g_t.s, lp_pt(14, 14)), LC_WHITE);
	lt_close(&g_t);
}

static void	progression_de_6_px(void)
{
	t_color	piste;

	h_true(lt_open(&g_t, 130, 20) == 0, "allocation");
	lt_clear(&g_t, LC_WHITE);
	luna_progress(&g_t.s, lp_rect(10, 7, 100, 6), 50);
	h_eq_u64("remplissage accent", gfx_get(&g_t.s, lp_pt(30, 10)), VT_ACCENT);
	piste = gfx_get(&g_t.s, lp_pt(90, 10));
	h_true((piste >> 16 & 255) >= 226 && (piste >> 16 & 255) <= 232,
		"piste : encre entre 10 et 12 % sur blanc");
	h_eq_u64("coin arrondi de rayon 3", gfx_get(&g_t.s, lp_pt(10, 7)),
		LC_WHITE);
	lt_close(&g_t);
}

static const t_rect	g_vides[] = {{0, 0, 0, 0}, {5, 5, -3, 4}, {5, 5, 4, -3},
{-5, -5, 0, 7}, {INT32_MIN, INT32_MIN, INT32_MAX, INT32_MAX},
{INT32_MAX, INT32_MAX, 5, 5}};

static void	focus_sur_rect_vide(void)
{
	uint64_t	avant;
	int32_t		i;

	h_true(lt_open(&g_t, 40, 20) == 0, "allocation");
	lt_clear(&g_t, LC_BLACK);
	avant = lt_hash(&g_t.s);
	i = 0;
	while (i < 6)
	{
		lp_rfocus(&g_t.s, g_vides[i], 8, VT_ACCENT);
		h_true(lt_hash(&g_t.s) == avant, "rect vide ou hors plage : rien");
		i++;
	}
	h_eq_u64("coin haut gauche intact", gfx_get(&g_t.s, lp_pt(0, 0)),
		LC_BLACK);
	h_true(lt_guard_ok(&g_t), "aucune ecriture hors de la surface");
	lt_close(&g_t);
}

int	main(void)
{
	h_begin("a17/ctrl_maquette");
	h_run("oracle maquette : radio coche, anneau 4 px", radio_coche_anneau_4px);
	h_run("oracle maquette : radio decoche, contour #8A93A8",
		radio_decoche_contour_gris);
	h_run("valeurs limites : barre de 6 px de haut", progression_de_6_px);
	h_run("valeurs limites : anneau de focus sur rect vide ou enorme",
		focus_sur_rect_vide);
	return (h_end());
}
