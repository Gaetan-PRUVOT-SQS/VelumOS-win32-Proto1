#ifndef SHELL_H
# define SHELL_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/ctl.h"
# include "velum/font.h"
# include "velum/gfx.h"
# include "velum/luna.h"
# include "velum/wm.h"
# include "../common/timefmt.h"
# include "../common/uiwin.h"
# include "desktop.h"
# include "sh_launch_apk.h"
# include "startmenu.h"
# include "taskbar.h"
# include "tasklist.h"

# define SH_CHILD_MAX 16
# define SH_USER_MAX 32
# define SH_LABEL_MAX 40
# define SH_NOTE_MAX 128
# define SHA_NONE 0
# define SHA_RUN 1
# define SHA_HALT 2
# define SHA_REBOOT 3
# define SHA_CLOSE 4
# define SH_HELLO_PATH "/system/bin/hello"
# define SH_APKRUN_PATH "/system/bin/apkrun"
# define SH_DEFAULT_USER "Utilisateur"
# define SH_WALL_COLOR 0xff3a6ea5u
# define SH_DLG_RUN 1
# define SH_DLG_POWER 2
# define SH_DLG_NOTE 3
# define SH_ID_EDIT 20
# define SH_ID_HALT 30
# define SH_ID_REBOOT 31
# define SH_GLASS_LAUNCHER_W 540
# define SH_GLASS_LAUNCHER_H 540
# define SH_DATE_MAX 11

typedef struct s_civil
{
	int64_t	y;
	int64_t	m;
	int64_t	d;
}	t_civil;

typedef struct s_cellctx
{
	struct s_shell	*sh;
	t_rect			cell;
	int				selected;
}	t_cellctx;

typedef struct s_shell
{
	t_wmhello		info;
	t_lunametrics	lm;
	t_wmwin			desk;
	t_wmwin			bar;
	t_wmwin			menu;
	uint32_t		*wall_px;
	t_surface		wall;
	uint32_t		*glass_px;
	size_t			glass_n;
	t_tasklist		tasks;
	t_startmenu		sm;
	t_smprogs		progs;
	t_smlayout		sml;
	t_lmodel		lmodel;
	t_tbregions		reg;
	t_rect			cells[DESK_ICONS];
	t_rect			btn[TASK_MAX];
	uint32_t		placed;
	uint32_t		shown;
	int32_t			selected;
	int32_t			hot_task;
	int				hot_start;
	int				menu_shown;
	t_click			last_click;
	t_deskstate		desk_state;
	char			user[SH_USER_MAX];
	char			clock[CLOCK_TEXT_MAX];
	char			date[SH_DATE_MAX];
	t_handle		timer;
	t_handle		children[SH_CHILD_MAX];
	uint32_t		nchildren;
	t_uiwin			dlg;
	int				dlg_kind;
	int				dlg_act;
	char			note[SH_NOTE_MAX];
	char			arg[CTL_TEXT_MAX];
	int				quit;
}	t_shell;

int		sh_boot(t_shell *sh, int argc, char **argv);
void	sh_shutdown(t_shell *sh);
int		sh_make_windows(t_shell *sh);
void	sh_layout(t_shell *sh);
void	sh_run(t_shell *sh);
void	sh_dispatch(t_shell *sh, const t_uimsg *m);
void	sh_on_key(t_shell *sh, const t_wmkey *k);
void	sh_on_mouse(t_shell *sh, const t_wmmouse *m);
void	sh_wall_make(t_shell *sh);
void	sh_wall_blit(t_shell *sh, t_rect r);
int		sh_wall_sync(t_shell *sh);
int		sh_theme_sync(t_shell *sh);
void	sh_desk_paint(t_shell *sh);
void	sh_desk_cell(t_shell *sh, int32_t idx);
void	sh_desk_cell_draw(t_shell *sh, int32_t idx);
void	sh_desk_mouse(t_shell *sh, const t_wmmouse *m);
void	sh_desk_key(t_shell *sh, uint32_t vk);
void	sh_desk_sync(t_shell *sh);
void	sh_desk_press(t_shell *sh, int32_t hit, const t_wmmouse *m);
void	sh_desk_move(t_shell *sh, const t_wmmouse *m);
void	sh_desk_release(t_shell *sh, const t_wmmouse *m);
void	sh_desk_trash_empty(t_shell *sh);
void	sh_desk_trash_open(t_shell *sh);
void	sh_bar_paint(t_shell *sh);
void	sh_bar_tray(t_shell *sh);
int		sh_bar_tray_hit(t_shell *sh, int32_t x, int32_t y);
t_color	sh_ink(void);
size_t	sh_glass_px(int32_t screen_w);
int		sh_glass_alloc(t_shell *sh, size_t px);
int		sh_glass_make(t_shell *sh);
void	sh_glass_free(t_shell *sh);
void	sh_bar_relayout(t_shell *sh);
void	sh_bar_mouse(t_shell *sh, const t_wmmouse *m);
void	sh_task_click(t_shell *sh, int32_t idx);
void	sh_menu_geo(t_shell *sh);
void	sh_menu_paint(t_shell *sh);
void	sh_menu_open(t_shell *sh);
void	sh_menu_close(t_shell *sh);
void	sh_menu_toggle(t_shell *sh);
void	sh_menu_mouse(t_shell *sh, const t_wmmouse *m);
void	sh_menu_key(t_shell *sh, uint32_t vk);
void	sh_menu_run(t_shell *sh, int cmd);
void	sh_launch_ui_refresh(t_shell *sh);
void	sh_launch_ui_open(t_shell *sh);
void	sh_launch_ui_note(t_shell *sh, uint32_t id);
int		sh_launch_ui_char(t_shell *sh, uint32_t codepoint);
int		sh_launch_ui_key(t_shell *sh, uint32_t vk);
int		sh_launch_ui_mouse(t_shell *sh, const t_wmmouse *m);
void	sh_date_fmt(uint64_t wall_ns, char *out, size_t size);
void	sh_clock_arm(t_shell *sh);
void	sh_clock_tick(t_shell *sh);
int		sh_launch(t_shell *sh, const char *path);
int		sh_launch_arg(t_shell *sh, const char *path, const char *arg);
void	sh_load_programs(t_shell *sh);
void	sh_launch_program(t_shell *sh, uint32_t cmd);
void	sh_reap(t_shell *sh);
void	sh_kill_children(t_shell *sh);
void	sh_logoff(t_shell *sh);
void	sh_power(t_shell *sh, uint32_t op);
void	sh_run_command(t_shell *sh, const char *text);
void	sh_desk_action(t_shell *sh, uint32_t action);
void	sh_dlg_open(t_shell *sh, int kind, const char *title);
void	sh_dlg_close(t_shell *sh);
int		sh_dlg_build(t_shell *sh, int kind);
void	sh_dlg_command(void *c, uint32_t code, void *user);
void	sh_dlg_finish(t_shell *sh);
void	sh_note(t_shell *sh, const char *title, const char *text);

#endif
