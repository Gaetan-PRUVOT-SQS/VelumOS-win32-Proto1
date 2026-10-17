#ifndef LUNA_DOCK_H
# define LUNA_DOCK_H

# include "velum/gfx.h"
# include "velum/theme.h"

# define LD_TRAY_PAD_L 14
# define LD_TRAY_PAD_R 6
# define LD_TRAY_GAP 14
# define LD_TRAY_LANG_W 17
# define LD_TRAY_DOTS_W 38
# define LD_TRAY_CLOCK_W 74
# define LD_TRAY_W 221
# define LD_STRIP_PAD_L 4

typedef struct s_traygeo
{
	t_rect	lang;
	t_rect	dots;
	t_rect	clock;
	t_rect	badge;
}	t_traygeo;

void	luna_tray_geo(t_rect tray, t_traygeo *out);

#endif
