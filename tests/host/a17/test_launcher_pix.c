#include "harness.h"
#include "velum/luna_launcher.h"
#include "velum/theme.h"
#include "lt_contrast.h"

static const t_launchertile	g_tiles[] = {
{"Bonjour", ICON_PROGRAM}, {"Console", ICON_RUN}};

static const t_color		g_veil[2][4] = {
{0xb8ffffff, 0x9effffff, 0x99ffffff, 0x8cffffff},
{0x14ffffff, 0x1affffff, 0x14ffffff, 0x14ffffff}};

static t_lt					g_t;

static int	near_mix(t_color got, t_color dst, t_color src)
{
	uint32_t	a;
	uint32_t	sh;
	uint32_t	want;
	int32_t		diff;

	a = src >> 24;
	sh = 0;
	while (sh < 24)
	{
		want = (((src >> sh) & 255u) * a + ((dst >> sh) & 255u) * (255u - a)
				+ 127u) / 255u;
		diff = (int32_t)((got >> sh) & 255u) - (int32_t)want;
		if (diff < -1 || diff > 1)
			return (0);
		sh += 8;
	}
	return (1);
}

static void	check_theme(t_lunatheme theme, bool reduced)
{
	t_launcher	l;
	t_color		*px;
	int32_t		st;

	luna_theme_set(theme);
	luna_transparency_set(reduced);
	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = 0;
	l.focus = LUNA_LAUNCHER_NONE;
	lt_clear(&g_t, 0xff6a8f4c);
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
	px = g_t.s.px;
	st = g_t.s.stride;
	h_true(near_mix(px[192 * st + 480], px[192 * st + 26],
			g_veil[theme][0]), "recherche : blanc 72 % clair, 8 % sombre");
	h_true(near_mix(px[350 * st + 90], px[350 * st + 26],
			g_veil[theme][1]), "tuile survolee : blanc 62 % clair, 10 % sombre");
	h_true(near_mix(px[374 * st + 300], px[373 * st + 300],
			g_veil[theme][2]), "filet de pied : blanc 60 % clair, 8 % sombre");
	h_true(near_mix(px[403 * st + 346], px[403 * st + 26],
			g_veil[theme][3]), "bouton de pied : blanc 55 % clair, 8 % sombre");
	h_true(near_mix(px[350 * st + 200], px[350 * st + 26], 0), "tuile au repos");
}

static void	pixels_all_modes(void)
{
	check_theme(LUNA_THEME_LIGHT, false);
	check_theme(LUNA_THEME_LIGHT, true);
	check_theme(LUNA_THEME_DARK, false);
	check_theme(LUNA_THEME_DARK, true);
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
}

static void	reduced_opaque(void)
{
	t_launcher	l;
	t_color		back;
	t_color		want;
	int32_t		k;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	luna_transparency_set(true);
	k = 0;
	while (k < 4)
	{
		luna_theme_set(k / 2);
		back = 0xff102030 + (t_color)k * 0x00305070;
		want = VT_OPAQUE_PANEL;
		if (k / 2)
			want = VT_OPAQUE_PANEL_D;
		lt_clear(&g_t, back);
		luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
		h_eq_u64("fond opaque, sans lien avec l'arriere-plan",
			g_t.s.px[365 * g_t.s.stride + 300], want);
		k++;
	}
	luna_theme_set(LUNA_THEME_LIGHT);
	luna_transparency_set(false);
}

static void	text_origins(void)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_NONE;
	lk_begin(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(12, 150, 540, 289));
	h_eq_i64("six textes", fk_spy()->n, 6);
	h_eq_i64("recherche : texte a 10 + 26 + 10 de la pilule",
		fk_spy()->rec[0].box.x, 12 + 18 + 46);
	h_eq_i64("Epinglees : retrait de 6", fk_spy()->rec[1].box.x, 12 + 18 + 6);
	h_eq_i64("nom : 10 apres l'avatar de 34", fk_spy()->rec[4].box.x,
		12 + 18 + 10 + 34 + 10);
}

int	main(void)
{
	if (lt_open(&g_t, 640, 480) != 0)
		return (1);
	h_begin("a17/lanceur_pixels");
	h_run("voiles et filets par theme, transparence normale et reduite",
		pixels_all_modes);
	h_run("transparence reduite : fond opaque du panneau", reduced_opaque);
	h_run("origine des textes du modele", text_origins);
	lt_close(&g_t);
	return (h_end());
}
