#ifndef LUNA_H
# define LUNA_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "gfx.h"

# define HT_NOWHERE 0
# define HT_CLIENT 1
# define HT_CAPTION 2
# define HT_SYSMENU 3
# define HT_MINBUTTON 8
# define HT_MAXBUTTON 9
# define HT_LEFT 10
# define HT_RIGHT 11
# define HT_TOP 12
# define HT_TOPLEFT 13
# define HT_TOPRIGHT 14
# define HT_BOTTOM 15
# define HT_BOTTOMLEFT 16
# define HT_BOTTOMRIGHT 17
# define HT_CLOSE 20

# define WS_CAPTION 0x0001
# define WS_SYSMENU 0x0002
# define WS_MINBOX 0x0004
# define WS_MAXBOX 0x0008
# define WS_SIZEBOX 0x0010
# define WS_POPUP 0x0020
# define WS_TOOLWINDOW 0x0040
# define WS_NOACTIVATE 0x0080
# define WS_TOPMOST 0x0100
# define WS_DESKTOP 0x0200
# define WS_APPBAR 0x0400
# define WS_FULLSCREEN 0x0800
# define WS_DEFAULT 0x001f

typedef enum e_lunastate
{
	LS_NORMAL = 0,
	LS_HOT,
	LS_PRESSED,
	LS_DISABLED,
	LS_DEFAULT,
	LS_FOCUSED
}	t_lunastate;

typedef enum e_iconid
{
	ICON_NONE = 0,
	ICON_LOGO,
	ICON_COMPUTER,
	ICON_FOLDER,
	ICON_DOCUMENT,
	ICON_RECYCLE,
	ICON_DRIVE,
	ICON_SETTINGS,
	ICON_RUN,
	ICON_SEARCH,
	ICON_HELP,
	ICON_POWER,
	ICON_USER,
	ICON_PROGRAM,
	ICON_IDS
}	t_iconid;

typedef enum e_lunatheme
{
	LUNA_THEME_LIGHT = 0,
	LUNA_THEME_DARK
}	t_lunatheme;

typedef struct s_lunametrics
{
	int32_t	caption_h;
	int32_t	frame_w;
	int32_t	frame_bottom;
	int32_t	btn_w;
	int32_t	btn_h;
	int32_t	taskbar_h;
	int32_t	start_w;
	int32_t	tray_w;
	int32_t	scroll_w;
	int32_t	menu_item_h;
	int32_t	icon_small;
	int32_t	icon_large;
}	t_lunametrics;

typedef struct s_lunawin
{
	t_rect		outer;
	uint32_t	style;
	const char	*title;
	t_iconid	icon;
	bool		active;
	bool		maximized;
	uint32_t	hot;
	uint32_t	pressed;
}	t_lunawin;

typedef struct s_lunabtn
{
	t_rect		r;
	t_lunastate	state;
	bool		is_default;
	bool		focus;
}	t_lunabtn;

void		luna_metrics(t_lunametrics *out);
void		luna_window_frame(t_surface *s, const t_lunawin *w);
t_rect		luna_window_client(const t_lunawin *w);
uint32_t	luna_hit_test(const t_lunawin *w, t_point p);
void		luna_button(t_surface *s, const t_lunabtn *b);
void		luna_checkbox(t_surface *s, t_point p, t_lunastate st, bool on);
void		luna_radio(t_surface *s, t_point p, t_lunastate st, bool on);
void		luna_edit_frame(t_surface *s, t_rect r, bool enabled);
void		luna_groupbox(t_surface *s, t_rect r, int32_t label_w);
void		luna_scrollbar(t_surface *s, t_rect r, t_lunastate st);
void		luna_progress(t_surface *s, t_rect r, uint32_t pct);
void		luna_menu_panel(t_surface *s, t_rect r);
void		luna_menu_item(t_surface *s, t_rect r, t_lunastate st);
void		luna_taskbar(t_surface *s, t_rect r);
void		luna_start_button(t_surface *s, t_rect r, t_lunastate st);
void		luna_task_button(t_surface *s, t_rect r, t_lunastate st);
void		luna_tray(t_surface *s, t_rect r);
void		luna_wallpaper(t_surface *s, t_rect r);
void		luna_logon_bg(t_surface *s, t_rect r);
void		luna_icon(t_surface *s, t_point p, t_iconid id, int32_t size);
void		luna_boot_draw(t_surface *s, uint32_t pct, uint32_t tick);
typedef enum e_glasskind
{
	LUNA_GLASS_DOCK = 0,
	LUNA_GLASS_LAUNCHER,
	LUNA_GLASS_PANEL,
	LUNA_GLASS_CARD,
	LUNA_GLASS_CHIP,
	LUNA_GLASS_FRAME,
	LUNA_GLASS_WINDOW,
	LUNA_GLASS_CONSOLE,
	LUNA_GLASS_FLOAT,
	LUNA_GLASS_KINDS
}	t_glasskind;

void		luna_glass_scratch(uint32_t *buf, size_t px);
size_t		luna_glass_need(t_rect r);
void		luna_glass(t_surface *s, t_rect r, t_glasskind kind);
void		luna_theme_set(t_lunatheme theme);
t_lunatheme	luna_theme_get(void);
void		luna_transparency_set(bool reduced);
bool		luna_transparency_reduced(void);
t_color		luna_color_accent(void);
t_color		luna_color_text(void);
t_color		luna_color_window(void);
t_color		luna_color_selection(void);

typedef struct s_lunascroll
{
	t_rect		r;
	int32_t		total;
	int32_t		page;
	int32_t		pos;
	t_lunastate	up;
	t_lunastate	down;
	t_lunastate	thumb;
}	t_lunascroll;

typedef struct s_scrollparts
{
	t_rect	up;
	t_rect	down;
	t_rect	track;
	t_rect	thumb;
}	t_scrollparts;

typedef struct s_taskbarlayout
{
	t_rect	start;
	t_rect	tasks;
	t_rect	tray;
}	t_taskbarlayout;

int32_t		luna_window_glass_bands(const t_lunawin *w, t_rect *out);
int32_t		luna_client_band(const t_lunawin *w);
void		luna_client_band_paint(t_surface *s, const t_lunawin *w,
				const t_surface *src, t_color fallback);
t_rect		luna_caption_rect(const t_lunawin *w);
t_rect		luna_button_rect(const t_lunawin *w, uint32_t ht);
void		luna_scrollbar_ex(t_surface *s, const t_lunascroll *sb);
void		luna_scroll_layout(const t_lunascroll *sb, t_scrollparts *out);
void		luna_tooltip(t_surface *s, t_rect r);
void		luna_taskbar_layout(t_rect bar, t_taskbarlayout *out);

# define LUNA_FOCUS_MARGIN 4

t_color		luna_button_ink(const t_lunabtn *b);
#endif
