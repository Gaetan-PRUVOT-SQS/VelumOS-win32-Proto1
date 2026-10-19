#include <stdio.h>
#include "harness.h"
#include "lt_contrast.h"
#include "velum/luna_colors.h"

typedef struct s_pair
{
	const char	*name;
	t_lunacolor	fg;
	t_lunacolor	bg;
	double		min;
}	t_pair;

static const t_pair	g_pair[] = {
{"texte sur fenetre", LUNA_COLOR_TEXT, LUNA_COLOR_WINDOW, 4.5},
{"texte sur panneau", LUNA_COLOR_TEXT, LUNA_COLOR_PANEL, 4.5},
{"texte 2 sur fenetre", LUNA_COLOR_TEXT_2, LUNA_COLOR_WINDOW, 4.5},
{"texte 2 sur panneau", LUNA_COLOR_TEXT_2, LUNA_COLOR_PANEL, 4.5},
{"legende sur fenetre", LUNA_COLOR_MUTED, LUNA_COLOR_WINDOW, 4.5},
{"legende sur panneau", LUNA_COLOR_MUTED, LUNA_COLOR_PANEL, 4.5},
{"danger sur fenetre", LUNA_COLOR_DANGER, LUNA_COLOR_WINDOW, 4.5},
{"danger sur panneau", LUNA_COLOR_DANGER, LUNA_COLOR_PANEL, 4.5},
{"lien accent sur fenetre", LUNA_COLOR_ACCENT_TEXT, LUNA_COLOR_WINDOW, 4.5},
{"accent (barre, radio) sur fenetre", LUNA_COLOR_ACCENT, LUNA_COLOR_WINDOW,
	3.0},
{"accent (barre, radio) sur panneau", LUNA_COLOR_ACCENT, LUNA_COLOR_PANEL,
	3.0},
{NULL, LUNA_COLOR_TEXT, LUNA_COLOR_TEXT, 0.0}};

static t_color	over(t_color fg, t_color bg)
{
	uint32_t	a;
	uint32_t	out;
	uint32_t	sh;
	uint32_t	f;
	uint32_t	b;

	a = fg >> 24;
	out = 0xff000000U;
	sh = 0;
	while (sh < 24)
	{
		f = (fg >> sh) & 0xffU;
		b = (bg >> sh) & 0xffU;
		out |= (((f * a + b * (255U - a) + 127U) / 255U) & 0xffU) << sh;
		sh += 8;
	}
	return (out);
}

static void	pairs_theme(const char *tag)
{
	char	name[96];
	size_t	i;

	i = 0;
	while (g_pair[i].name != NULL)
	{
		snprintf(name, sizeof(name), "%s : %s", tag, g_pair[i].name);
		lk_check_min(name, lk_ratio(luna_color(g_pair[i].fg),
				luna_color(g_pair[i].bg)), g_pair[i].min);
		i++;
	}
}

static void	selection_theme(const char *tag)
{
	t_color	bg;
	char	name[96];

	bg = over(luna_color(LUNA_COLOR_SELECTION_BG),
			luna_color(LUNA_COLOR_WINDOW));
	snprintf(name, sizeof(name), "%s : texte sur ligne selectionnee", tag);
	lk_check(name, lk_ratio(luna_color(LUNA_COLOR_TEXT), bg));
	snprintf(name, sizeof(name), "%s : blanc sur selection pleine", tag);
	lk_check(name, lk_ratio(LC_WHITE, luna_color_selection()));
}

static void	theme_pairs(void)
{
	luna_theme_set(LUNA_THEME_LIGHT);
	pairs_theme("clair");
	selection_theme("clair");
	luna_theme_set(LUNA_THEME_DARK);
	pairs_theme("sombre");
	selection_theme("sombre");
	luna_theme_set(LUNA_THEME_LIGHT);
}

int	main(void)
{
	h_begin("a17/theme_contrast");
	h_run("P25 paires texte et fond de la maquette, WCAG AA, 2 themes",
		theme_pairs);
	return (h_end());
}
