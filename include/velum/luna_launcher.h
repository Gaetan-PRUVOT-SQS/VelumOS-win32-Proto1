#ifndef LUNA_LAUNCHER_H
# define LUNA_LAUNCHER_H

# include <stdint.h>
# include "velum/luna.h"

# define LUNA_LAUNCHER_WIDTH 540
# define LUNA_LAUNCHER_HEIGHT 289
# define LUNA_LAUNCHER_MARGIN_X 12
# define LUNA_LAUNCHER_TILES 4
# define LUNA_LAUNCHER_NONE -1
# define LUNA_LAUNCHER_LOGOFF 4
# define LUNA_LAUNCHER_POWER 5

typedef struct s_launchertile
{
	const char	*label;
	t_iconid	icon;
}	t_launchertile;

typedef struct s_launcher
{
	const char				*user;
	const t_launchertile	*tiles;
	int32_t					ntiles;
	int32_t					hot;
	int32_t					focus;
}	t_launcher;

typedef struct s_launcherlayout
{
	t_rect	panel;
	t_rect	search;
	t_rect	title;
	t_rect	tile[LUNA_LAUNCHER_TILES];
	t_rect	sep;
	t_rect	avatar;
	t_rect	user;
	t_rect	logoff;
	t_rect	poweroff;
	int32_t	ntiles;
}	t_launcherlayout;

void	luna_launcher_layout(t_rect panel, int32_t ntiles,
			t_launcherlayout *out);
int32_t	luna_launcher_hit(const t_launcherlayout *l, int32_t x, int32_t y);
t_rect	luna_launcher_place(int32_t screen_h, int32_t bar_h);
t_rect	luna_launcher_place_in(int32_t screen_w, int32_t screen_h,
			int32_t bar_h);
void	luna_launcher(t_surface *s, const t_launcher *l, t_rect panel);

#endif
