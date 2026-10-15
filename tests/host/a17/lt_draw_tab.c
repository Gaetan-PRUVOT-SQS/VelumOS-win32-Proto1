#include "lt_draw.h"

static const t_ltdraw	g_draws[] = {
{"frame_active", lt_d_frame_active},
{"frame_inactive", lt_d_frame_inactive},
{"frame_max", lt_d_frame_max},
{"frame_tool", lt_d_frame_tool},
{"frame_dialog", lt_d_frame_dialog},
{"button", lt_d_button},
{"check", lt_d_check},
{"edit", lt_d_edit},
{"group", lt_d_group},
{"scroll", lt_d_scroll},
{"scroll_ex", lt_d_scroll_ex},
{"progress", lt_d_progress},
{"menu", lt_d_menu},
{"tooltip", lt_d_tooltip},
{"taskbar", lt_d_taskbar},
{"start", lt_d_start},
{"task", lt_d_task},
{"startmenu", lt_d_startmenu},
{"wallpaper", lt_d_wallpaper},
{"logon", lt_d_logon},
{"icons", lt_d_icons},
{"boot", lt_d_boot}
};

static const int		g_draw_count = 22;

const t_ltdraw	*lt_draw_get(int i)
{
	if (i < 0 || i >= g_draw_count)
		return (NULL);
	return (&g_draws[i]);
}

int	lt_draw_count(void)
{
	return (g_draw_count);
}
