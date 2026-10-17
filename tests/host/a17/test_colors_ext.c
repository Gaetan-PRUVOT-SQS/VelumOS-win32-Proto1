#include "harness.h"
#include "velum/theme.h"
#include "velum/luna_colors.h"
#include "lt_contrast.h"

static const t_color	g_light[LUNA_COLORS] = {VT_INK, VT_INK_2, VT_MUTED,
	VT_DIM, VT_ACCENT, VT_ACCENT_HI, VT_ACCENT, VT_DANGER,
	VT_OPAQUE_CONTENT, VT_OPAQUE_PANEL, VT_ACCENT_SEL_BG,
	VT_ACCENT_SEL_RING};
static const t_color	g_dark[LUNA_COLORS] = {VT_INK_D, VT_INK_2_D,
	VT_MUTED_D, VT_DIM_D, VT_ACCENT_HI, VT_ACCENT_HI, VT_ACCENT_TEXT_DARK,
	VT_DANGER_D, VT_OPAQUE_CONTENT_D, VT_OPAQUE_PANEL_D,
	VT_ACCENT_SEL_BG_D, VT_ACCENT_SEL_RING_D};

static void	check_theme(t_lunatheme th, const t_color *want, const char *what)
{
	int	id;

	luna_theme_set(th);
	id = 0;
	while (id < LUNA_COLORS)
	{
		h_eq_u64(what, luna_color((t_lunacolor)id), want[id]);
		id++;
	}
}

static void	colors_tokens(void)
{
	check_theme(LUNA_THEME_LIGHT, g_light, "jeton clair de la maquette");
	check_theme(LUNA_THEME_DARK, g_dark, "jeton sombre de la maquette");
	luna_theme_set(LUNA_THEME_LIGHT);
}

static void	colors_edges(void)
{
	luna_theme_set(LUNA_THEME_DARK);
	h_eq_u64("id negatif retombe sur le texte",
		luna_color((t_lunacolor)-1), luna_color_text());
	h_eq_u64("id egal au nombre de couleurs",
		luna_color(LUNA_COLORS), luna_color_text());
	h_eq_u64("id enorme", luna_color((t_lunacolor)INT32_MAX),
		luna_color_text());
	h_eq_u64("accent nomme", luna_color_accent(),
		luna_color(LUNA_COLOR_ACCENT));
	h_eq_u64("texte 2", luna_color_text_2(), luna_color(LUNA_COLOR_TEXT_2));
	h_eq_u64("attenue", luna_color_muted(), luna_color(LUNA_COLOR_MUTED));
	h_eq_u64("danger", luna_color_danger(), luna_color(LUNA_COLOR_DANGER));
	h_eq_u64("panneau", luna_color_panel(), luna_color(LUNA_COLOR_PANEL));
	h_eq_u64("fenetre", luna_color_window(), luna_color(LUNA_COLOR_WINDOW));
	luna_theme_set(LUNA_THEME_LIGHT);
}

static void	colors_contrast(void)
{
	luna_theme_set(LUNA_THEME_LIGHT);
	lk_check("clair : texte sur fenetre", lk_ratio(
			luna_color(LUNA_COLOR_TEXT), luna_color(LUNA_COLOR_WINDOW)));
	lk_check("clair : attenue sur fenetre", lk_ratio(
			luna_color(LUNA_COLOR_MUTED), luna_color(LUNA_COLOR_WINDOW)));
	luna_theme_set(LUNA_THEME_DARK);
	lk_check("sombre : texte sur fenetre", lk_ratio(
			luna_color(LUNA_COLOR_TEXT), luna_color(LUNA_COLOR_WINDOW)));
	lk_check("sombre : attenue sur fenetre", lk_ratio(
			luna_color(LUNA_COLOR_MUTED), luna_color(LUNA_COLOR_WINDOW)));
	lk_check("sombre : accent sur fenetre", lk_ratio(
			luna_color(LUNA_COLOR_ACCENT_TEXT), luna_color(LUNA_COLOR_WINDOW)));
	luna_theme_set(LUNA_THEME_LIGHT);
}

int	main(void)
{
	h_begin("a17/colors_ext");
	h_run("P1 jetons clair et sombre de la maquette", colors_tokens);
	h_run("valeurs limites de l'id et accesseurs nommes", colors_edges);
	h_run("contraste du texte sur la fenetre, deux themes", colors_contrast);
	return (h_end());
}
