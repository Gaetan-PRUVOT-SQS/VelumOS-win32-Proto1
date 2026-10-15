#ifndef CTL_H
# define CTL_H

# include <stdbool.h>
# include <stdint.h>
# include "font.h"
# include "gfx.h"
# include "luna.h"
# include "wm.h"

# define CTL_TEXT_MAX 256
# define CTL_VISIBLE 0x01
# define CTL_ENABLED 0x02
# define CTL_FOCUSABLE 0x04
# define CTL_CHECKED 0x08
# define CTL_DEFAULT 0x10
# define CTL_MULTILINE 0x20
# define CTL_PASSWORD 0x40
# define CTL_NOBACK 0x80
# define CN_CLICKED 1
# define CN_CHANGED 2
# define CN_SELECT 3
# define CN_ENTER 4
# define CN_DBLCLK 5
# define CN_SCROLL 6

typedef enum e_ctltype
{
	CT_PANEL = 0,
	CT_LABEL,
	CT_BUTTON,
	CT_CHECK,
	CT_RADIO,
	CT_GROUP,
	CT_EDIT,
	CT_LIST,
	CT_SCROLL,
	CT_PROGRESS,
	CT_MENU,
	CT_IMAGE,
	CT_TYPES
}	t_ctltype;

typedef void	(*t_ctlcb)(void *c, uint32_t code, void *user);

typedef struct s_ctl
{
	uint32_t		type;
	uint32_t		id;
	uint32_t		flags;
	uint32_t		state;
	t_rect			rect;
	char			text[CTL_TEXT_MAX];
	struct s_ctl	*parent;
	struct s_ctl	*first;
	struct s_ctl	*next;
	t_ctlcb			cb;
	void			*user;
	void			*priv;
	uint32_t		font;
}	t_ctl;

typedef struct s_ctlroot
{
	t_ctl		*root;
	t_ctl		*focus;
	t_ctl		*hot;
	t_ctl		*capture;
	t_surface	*surface;
	t_region	dirty;
	t_ctlcb		command;
	void		*user;
}	t_ctlroot;

typedef struct s_ctlspec
{
	uint32_t	type;
	uint32_t	id;
	uint32_t	flags;
	t_rect		rect;
	const char	*text;
}	t_ctlspec;

int		ctl_root_init(t_ctlroot *r, t_surface *surface, t_ctlcb command);
void	ctl_root_destroy(t_ctlroot *r);
t_ctl	*ctl_add(t_ctlroot *r, t_ctl *parent, const t_ctlspec *spec);
void	ctl_remove(t_ctlroot *r, t_ctl *c);
t_ctl	*ctl_find(t_ctlroot *r, uint32_t id);
void	ctl_set_text(t_ctlroot *r, t_ctl *c, const char *text);
void	ctl_set_flag(t_ctlroot *r, t_ctl *c, uint32_t flag, bool on);
void	ctl_invalidate(t_ctlroot *r, t_ctl *c);
void	ctl_focus(t_ctlroot *r, t_ctl *c);
int		ctl_list_add(t_ctlroot *r, t_ctl *list, const char *item);
int		ctl_list_selected(const t_ctl *list);
void	ctl_progress_set(t_ctlroot *r, t_ctl *c, uint32_t pct);
bool	ctl_mouse(t_ctlroot *r, const t_wmmouse *m);
bool	ctl_key(t_ctlroot *r, const t_inpevent *ev);
t_rect	ctl_paint(t_ctlroot *r);

# define CTL_ALIGN_CENTER 0x100
# define CTL_ALIGN_RIGHT 0x200
# define CTL_ELLIPSIS 0x400
# define CTL_BOLD 0x800
# define CTL_ID_OK 1
# define CTL_ID_CANCEL 2
# define CN_CLOSED 7
# define CTL_MI_DISABLED 0x01
# define CTL_MI_CHECKED 0x02
# define CTL_MI_SEPARATOR 0x04
# define CTL_MENU_ITEMS_MAX 64
# define CTL_MENU_LEVELS_MAX 4
# define CTL_LIST_ITEMS_MAX 65536
# define CTL_DEPTH_MAX 32
# define CTL_DBLCLK_NS 500000000

typedef int64_t	(*t_ctlclock)(void);

typedef struct s_ctlmenuitem
{
	const char					*text;
	uint32_t					id;
	uint32_t					flags;
	const struct s_ctlmenuitem	*sub;
	uint32_t					nsub;
}	t_ctlmenuitem;

typedef struct s_ctlmenuspec
{
	uint32_t			id;
	t_point				at;
	const t_ctlmenuitem	*items;
	uint32_t			count;
}	t_ctlmenuspec;

void	ctl_set_clock(t_ctlroot *r, t_ctlclock now);
int		ctl_set_rect(t_ctlroot *r, t_ctl *c, t_rect rect);
int		ctl_clip_set(const char *utf8, uint32_t len);
int		ctl_clip_get(char *out, uint32_t cap);
void	ctl_clip_clear(void);
int		ctl_edit_set_limit(t_ctlroot *r, t_ctl *c, uint32_t max_chars);
int		ctl_list_count(const t_ctl *list);
int		ctl_list_text(const t_ctl *list, int index, char *out, uint32_t cap);
int		ctl_list_select(t_ctlroot *r, t_ctl *list, int index);
int		ctl_list_remove(t_ctlroot *r, t_ctl *list, int index);
void	ctl_list_clear(t_ctlroot *r, t_ctl *list);
void	ctl_scroll_set(t_ctlroot *r, t_ctl *c, int max, int page);
int		ctl_scroll_pos(const t_ctl *c);
void	ctl_scroll_move(t_ctlroot *r, t_ctl *c, int pos);
t_ctl	*ctl_menu_popup(t_ctlroot *r, const t_ctlmenuspec *spec);
int		ctl_menu_result(const t_ctl *menu);
void	ctl_menu_close(t_ctlroot *r, t_ctl *menu);

# define CTL_FONT_AUTO FONT_IDS

void	ctl_set_font(t_ctlroot *r, t_ctl *c, t_fontid font);

#endif
