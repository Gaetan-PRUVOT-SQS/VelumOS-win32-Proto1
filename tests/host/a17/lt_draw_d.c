#include "lt_draw.h"
#include "velum/luna_launcher.h"

static const t_launchertile	g_tiles[] = {
{"Ex\xc3\xa9" "cuter...", ICON_RUN}, {"Bonjour", ICON_PROGRAM},
{"Libelle tres long qui depasse la tuile", ICON_PROGRAM},
{NULL, ICON_NONE}};

void	lt_d_start(t_surface *s, t_rect r)
{
	luna_start_button(s, r, LS_NORMAL);
	luna_start_button(s, r, LS_HOT);
	luna_start_button(s, r, LS_PRESSED);
}

void	lt_d_task(t_surface *s, t_rect r)
{
	luna_task_button(s, r, LS_NORMAL);
	luna_task_button(s, r, LS_HOT);
	luna_task_button(s, r, LS_PRESSED);
	luna_task_button(s, r, LS_DEFAULT);
}

void	lt_d_startmenu(t_surface *s, t_rect r)
{
	t_launcher	l;

	l.user = "Nom";
	l.tiles = g_tiles;
	l.ntiles = LUNA_LAUNCHER_TILES;
	l.hot = 1;
	l.focus = 0;
	luna_launcher(s, &l, r);
	l.user = NULL;
	l.hot = LUNA_LAUNCHER_NONE;
	l.focus = LUNA_LAUNCHER_LOGOFF;
	luna_launcher(s, &l, r);
	l.user = "Un nom d'utilisateur beaucoup trop long";
	l.tiles = NULL;
	l.ntiles = 7;
	l.focus = LUNA_LAUNCHER_POWER;
	luna_launcher(s, &l, r);
	luna_launcher(s, NULL, r);
}

void	lt_d_wallpaper(t_surface *s, t_rect r)
{
	luna_wallpaper(s, r);
}

void	lt_d_logon(t_surface *s, t_rect r)
{
	luna_logon_bg(s, r);
}
