#ifndef LT_DRAW_H
# define LT_DRAW_H

# include "lt.h"
# include "lt_geo.h"

typedef void	(*t_ltfn)(t_surface *s, t_rect r);

typedef struct s_ltdraw
{
	const char	*name;
	t_ltfn		fn;
}	t_ltdraw;

const t_ltdraw	*lt_draw_get(int i);
int				lt_draw_count(void);
void			lt_d_frame_active(t_surface *s, t_rect r);
void			lt_d_frame_inactive(t_surface *s, t_rect r);
void			lt_d_frame_max(t_surface *s, t_rect r);
void			lt_d_frame_tool(t_surface *s, t_rect r);
void			lt_d_frame_dialog(t_surface *s, t_rect r);
void			lt_d_button(t_surface *s, t_rect r);
void			lt_d_check(t_surface *s, t_rect r);
void			lt_d_edit(t_surface *s, t_rect r);
void			lt_d_group(t_surface *s, t_rect r);
void			lt_d_scroll(t_surface *s, t_rect r);
void			lt_d_scroll_ex(t_surface *s, t_rect r);
void			lt_d_progress(t_surface *s, t_rect r);
void			lt_d_menu(t_surface *s, t_rect r);
void			lt_d_tooltip(t_surface *s, t_rect r);
void			lt_d_taskbar(t_surface *s, t_rect r);
void			lt_d_start(t_surface *s, t_rect r);
void			lt_d_task(t_surface *s, t_rect r);
void			lt_d_startmenu(t_surface *s, t_rect r);
void			lt_d_wallpaper(t_surface *s, t_rect r);
void			lt_d_logon(t_surface *s, t_rect r);
void			lt_d_icons(t_surface *s, t_rect r);
void			lt_d_boot(t_surface *s, t_rect r);

#endif
