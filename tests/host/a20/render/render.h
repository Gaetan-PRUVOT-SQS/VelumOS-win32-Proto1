#ifndef RENDER_H
# define RENDER_H

# include <stdint.h>
# include "velum/abi/abi_types.h"
# include "velum/wm.h"
# include "uiwin.h"
# include "shell.h"
# include "logon.h"

# define FWM_MAX 16
# define FOS_SPAWN_MAX 16
# define FOS_TEXT_MAX 96
# define SCREEN_W 1024
# define SCREEN_H 768
# define WALL_NS 1791209220000000000ull
# define FOS_T0 1000000000000ull

typedef struct s_fwin
{
	uint32_t	id;
	uint32_t	style;
	uint32_t	state;
	t_rect		rect;
	char		title[WM_TITLE_MAX];
	t_wmwin		*w;
	uint32_t	presents;
}	t_fwin;

typedef struct s_infospec
{
	uint32_t	id;
	uint32_t	style;
	uint32_t	state;
	uint32_t	active;
	const char	*title;
}	t_infospec;

typedef struct s_fakewm
{
	t_fwin		win[FWM_MAX];
	uint32_t	count;
	uint32_t	next_id;
	uint32_t	activated;
	uint32_t	captured;
	uint32_t	state_id;
	uint32_t	state_value;
	int			subscribed;
}	t_fakewm;

typedef struct s_fakeos
{
	uint64_t	now;
	char		path[FOS_SPAWN_MAX][FOS_TEXT_MAX];
	char		arg[FOS_SPAWN_MAX][FOS_TEXT_MAX];
	uint32_t	flags[FOS_SPAWN_MAX];
	uint32_t	nspawn;
	uint32_t	killed;
	uint32_t	timer_armed;
	int			power_op;
	char		last_log[FOS_TEXT_MAX];
	const char	*users_file;
}	t_fakeos;

extern t_fakewm	g_fwm;
extern t_fakeos	g_fos;

void	render_reset(const char *users_file);
t_fwin	*fwm_find(const char *title);
t_fwin	*fwm_by_id(uint32_t id);
void	fwm_compose(t_surface *screen);
int		ppm_write(const t_surface *s, const char *path);
void	msg_mouse(t_uimsg *m, uint32_t win, uint32_t type, t_point p);
void	msg_button(t_uimsg *m, uint32_t win, uint32_t type, t_point p);
void	msg_key(t_uimsg *m, uint32_t win, uint32_t type, uint32_t code);
void	msg_info(t_uimsg *m, uint32_t type, const t_infospec *i);
int		px_near(const t_surface *s, t_point p, uint32_t rgb, int tol);

void	render_shot(const char *name);
t_point	rect_center(t_rect r);
void	drv_menu_item(t_shell *sh, int idx);
void	drv_move(t_shell *sh, uint32_t win, t_point p);
void	drv_click(t_shell *sh, uint32_t win, t_point p);
void	drv_key(t_shell *sh, uint32_t win, uint32_t code);
void	drv_type(t_shell *sh, uint32_t win, const char *text);
void	lgd_click(t_logon *lg, t_point p);
t_point	lgd_tile(const t_logon *lg, uint32_t i);
void	lgd_press_shutdown(t_logon *lg);
void	lgd_key(t_logon *lg, uint32_t code);
void	lgd_type(t_logon *lg, const char *text);

#endif
