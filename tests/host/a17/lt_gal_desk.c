#include "lt_gal.h"
#include "velum/luna_launcher.h"

static const t_launchertile	g_gal_tiles[] = {
{"Ex\xc3\xa9" "cuter...", ICON_RUN}, {"Bonjour", ICON_PROGRAM}};

static void	gal_bar(t_surface *s, t_rect r)
{
	t_taskbarlayout	lay;
	t_rect			t;
	int32_t			i;

	luna_taskbar(s, r);
	luna_taskbar_layout(r, &lay);
	luna_start_button(s, lay.start, LS_NORMAL);
	luna_tray(s, lay.tray);
	i = 0;
	while (i < 4)
	{
		t = lp_rect(lay.tasks.x + i * 160, lay.tasks.y + 3, 156, 24);
		luna_task_button(s, t, (i == 0) * LS_PRESSED + (i == 2) * LS_HOT);
		luna_icon(s, lp_pt(t.x + 4, t.y + 4), ICON_PROGRAM, 16);
		i++;
	}
}

static void	gal_launcher(t_surface *s, t_rect r)
{
	t_launcher	l;
	t_rect		menu;

	menu = luna_launcher_place(r.h, 30);
	menu.x += r.x;
	menu.y += r.y;
	l.user = "Utilisateur";
	l.tiles = g_gal_tiles;
	l.ntiles = 2;
	l.hot = 1;
	l.focus = 0;
	luna_launcher(s, &l, menu);
}

void	lt_gal_desktop(t_surface *s, t_rect r)
{
	luna_wallpaper(s, r);
	gal_launcher(s, r);
	gal_bar(s, lp_rect(r.x, r.y + r.h - 30, r.w, 30));
}

void	lt_gal_icons(t_surface *s, int32_t x, int32_t y)
{
	int32_t	id;

	id = 1;
	while (id < ICON_IDS)
	{
		luna_icon(s, lp_pt(x + (id - 1) * 56, y), (t_iconid)id, 48);
		luna_icon(s, lp_pt(x + (id - 1) * 56 + 8, y + 56), (t_iconid)id, 32);
		luna_icon(s, lp_pt(x + (id - 1) * 56 + 16, y + 96), (t_iconid)id, 16);
		id++;
	}
}
