#include "harness.h"
#include "velum/luna_launcher.h"
#include "lt.h"
#include "luna_int.h"

static const t_launchertile	g_tiles[] = {
{"Ex\xc3\xa9" "cuter...", ICON_RUN}, {"Bonjour", ICON_PROGRAM},
{"Console", ICON_RUN}, {"Fichiers", ICON_FOLDER}};

static void	render(t_lunatheme theme, const char *path)
{
	t_lt		t;
	t_launcher	l;

	luna_theme_set(theme);
	h_true(lt_open(&t, 1280, 800) == 0, "allocation");
	luna_wallpaper(&t.s, lp_rect(0, 0, 1280, 800));
	l.user = "Utilisateur";
	l.tiles = g_tiles;
	l.ntiles = 4;
	l.hot = 0;
	l.focus = LUNA_LAUNCHER_NONE;
	luna_launcher(&t.s, &l, luna_launcher_place(800, 80));
	lt_out_dir();
	h_true(lt_png(path, &t.s, 1) == 0, "ecriture png");
	lt_close(&t);
	luna_theme_set(LUNA_THEME_LIGHT);
}

static void	render_both(void)
{
	render(LUNA_THEME_LIGHT, "build/a17/lanceur_clair.png");
	render(LUNA_THEME_DARK, "build/a17/lanceur_sombre.png");
}

int	main(void)
{
	h_begin("a17/lanceur_png");
	h_run("rendu clair et sombre 1280x800", render_both);
	return (h_end());
}
