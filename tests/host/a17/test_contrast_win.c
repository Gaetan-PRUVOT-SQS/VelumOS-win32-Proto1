#include "harness.h"
#include "lt_contrast.h"
#include "velum/luna_launcher.h"

static t_lt					g_t;

static const t_launchertile	g_tiles[] = {
{"Ex\xc3\xa9" "cuter...", ICON_RUN}, {"Bonjour", ICON_PROGRAM}};

static void	win_title(void)
{
	uint32_t	tool;

	tool = WS_CAPTION | WS_SYSMENU | WS_TOOLWINDOW;
	lk_check("titre fenetre active", lk_title(&g_t, WS_DEFAULT, true));
	lk_check("titre fenetre inactive", lk_title(&g_t, WS_DEFAULT, false));
	lk_check("titre outil actif", lk_title(&g_t, tool, true));
	lk_check("titre outil inactif", lk_title(&g_t, tool, false));
}

static void	win_startmenu(void)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = 2;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = 0;
	lk_begin(&g_t, LC_BLACK);
	luna_launcher(&g_t.s, &l, lp_rect(0, 0, LUNA_LAUNCHER_WIDTH,
			LUNA_LAUNCHER_HEIGHT));
	h_eq_i64("six textes dans le lanceur", fk_spy()->n, 6);
	lk_check("lanceur, recherche, titre, tuiles, nom et pied",
		lk_spied(&g_t.s));
	lk_begin(&g_t, LC_BLACK);
	luna_boot_draw(&g_t.s, 50, 0);
	lk_check("texte de l'ecran de demarrage", lk_spied(&g_t.s));
}

static void	win_flat(void)
{
	t_rect	r;

	r = lp_rect(0, 0, 120, 24);
	lk_check("texte sur fond de fenetre",
		lk_ratio(luna_color_text(), luna_color_window()));
	lk_check("texte blanc sur selection",
		lk_ratio(LC_WHITE, luna_color_selection()));
	lk_begin(&g_t, LC_BLACK);
	luna_menu_panel(&g_t.s, lp_rect(0, 0, 120, 60));
	lk_check("texte sur panneau de menu", lk_worst(&g_t.s,
			lp_rect(4, 4, 112, 52), luna_color_text()));
	lk_begin(&g_t, LC_BLACK);
	luna_menu_item(&g_t.s, r, LS_HOT);
	lk_check("entree de menu survolee", lk_worst(&g_t.s, r, LC_WHITE));
	lk_begin(&g_t, LC_BLACK);
	luna_tooltip(&g_t.s, r);
	lk_check("infobulle", lk_worst(&g_t.s, lp_rect(2, 2, 116, 20),
			luna_color_text()));
}

static void	win_button(void)
{
	static const char	*name[3] = {"bouton normal", "bouton survole",
		"bouton enfonce"};
	t_lunabtn			b;
	int					st;

	b.r = lp_rect(0, 0, 75, 23);
	b.is_default = false;
	b.focus = false;
	st = LS_NORMAL;
	while (st <= LS_PRESSED)
	{
		b.state = (t_lunastate)st;
		lk_begin(&g_t, LC_BLACK);
		luna_button(&g_t.s, &b);
		lk_check(name[st], lk_worst(&g_t.s, lk_band(lp_rect(6, 0, 63, 23),
					LK_UI), luna_color_text()));
		st++;
	}
}

int	main(void)
{
	if (lt_open(&g_t, 640, 480) != 0)
		return (1);
	h_begin("a17/contrast_win");
	h_run("P25 titres de fenetre", win_title);
	h_run("menu demarrer et ecran de demarrage", win_startmenu);
	h_run("couleurs de base, menus, infobulle", win_flat);
	h_run("boutons", win_button);
	lt_close(&g_t);
	return (h_end());
}
