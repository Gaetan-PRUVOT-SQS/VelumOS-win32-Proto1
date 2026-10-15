#include "shell.h"

static t_rect	menu_place(const t_shell *sh)
{
	return (luna_launcher_place((int32_t)sh->info.screen_h,
			sh->lm.taskbar_h));
}

void	sh_menu_geo(t_shell *sh)
{
	t_rect	panel;

	panel = menu_place(sh);
	panel.x = 0;
	panel.y = 0;
	luna_launcher_layout(panel, (int32_t)sm_tiles(), &sh->sml);
}

static void	backdrop(t_shell *sh)
{
	t_rect	at;
	t_blit	b;

	at = menu_place(sh);
	b.dst = &sh->menu.surface;
	b.dr = rect_make(0, 0, at.w, at.h);
	if (!sh->wall_px)
	{
		gfx_fill(b.dst, b.dr, SH_WALL_COLOR);
		return ;
	}
	b.src = &sh->wall;
	b.sr = at;
	gfx_blit(&b);
}

static void	fill_tiles(t_launchertile *tiles, int32_t n)
{
	const t_smitem	*it;
	int32_t			i;

	i = 0;
	while (i < n && i < LUNA_LAUNCHER_TILES)
	{
		it = sm_item((uint32_t)i);
		tiles[i].label = it->label;
		tiles[i].icon = (t_iconid)it->icon;
		i++;
	}
}

void	sh_menu_paint(t_shell *sh)
{
	t_launchertile	tiles[LUNA_LAUNCHER_TILES];
	t_launcher		l;

	if (!sh->menu.id)
		return ;
	sh_menu_geo(sh);
	backdrop(sh);
	fill_tiles(tiles, sh->sml.ntiles);
	l.user = sh->user;
	l.tiles = tiles;
	l.ntiles = sh->sml.ntiles;
	l.hot = sh->sm.hot;
	l.focus = sh->sm.focus;
	luna_launcher(&sh->menu.surface, &l, sh->sml.panel);
	wmc_present(&sh->menu, NULL, 0);
}
