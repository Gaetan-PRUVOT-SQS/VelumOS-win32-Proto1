#ifndef CTL_INT_H
# define CTL_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/ctl.h"
# include "velum/err.h"
# include "velum/libk.h"

# define CTL_FLAGS_ALL 0xfff
# define CTL_FLAGS_VISUAL 0xffb
# define CTL_COORD_MAX 1048576
# define CTL_BOX 13
# define CTL_GAP 4
# define CTL_PAD 3
# define CTL_ROW_PAD 2
# define CTL_FRAME 2
# define CTL_RING_W 2
# define CTL_EDIT_INSET 3
# define CTL_WHEEL_ROWS 3
# define CTL_DBLCLK_DIST 4
# define CTL_ARROW 4
# define CTL_THUMB_MIN 8
# define CTL_SCROLL_MAX 1073741824
# define CTL_MENU_BORDER 2
# define CTL_MENU_GUTTER 22
# define CTL_MENU_ARROW 16
# define CTL_MENU_PAD 4
# define CTL_ST_HOT 0x01
# define CTL_ST_PRESSED 0x02
# define CTL_ST_CLOSED 0x04
# define CTL_MEV_NONE 0
# define CTL_MEV_MOVE 1
# define CTL_MEV_DOWN 2
# define CTL_MEV_UP 3
# define CTL_MEV_WHEEL 4
# define CTL_SBZ_NONE 0
# define CTL_SBZ_DEC 1
# define CTL_SBZ_INC 2
# define CTL_SBZ_PAGE_DEC 3
# define CTL_SBZ_PAGE_INC 4
# define CTL_SBZ_THUMB 5
# define CTL_SBR_NONE 0
# define CTL_SBR_USED 1
# define CTL_SBR_MOVED 2
# define CTL_EDR_USED 1
# define CTL_EDR_TEXT 2
# define CTL_EDR_VIEW 4
# define CTL_EDR_ENTER 8
# define CTL_VK_A 0x41
# define CTL_VK_C 0x43
# define CTL_VK_V 0x56
# define CTL_VK_X 0x58

typedef struct s_mouseev
{
	uint32_t	kind;
	t_point		at;
	int32_t		wheel;
	bool		dbl;
}	t_mouseev;

typedef struct s_scrollst
{
	int32_t		pos;
	int32_t		max;
	int32_t		page;
	int32_t		grab;
	uint32_t	zone;
}	t_scrst;

typedef struct s_sblayout
{
	t_rect	dec;
	t_rect	inc;
	t_rect	track;
	t_rect	thumb;
	bool	vertical;
}	t_sblo;

typedef struct s_sbctx
{
	t_ctlroot	*r;
	t_ctl		*c;
	t_scrst		*st;
	t_rect		bar;
}	t_sbctx;

typedef struct s_ctltext
{
	t_rect			box;
	const char		*text;
	uint32_t		flags;
	t_color			color;
	bool			enabled;
	bool			mnemonic;
	const t_font	*font;
}	t_ctltext;

typedef struct s_textrun
{
	const t_font	*font;
	char			buf[CTL_TEXT_MAX];
	int32_t			len;
	int32_t			cut;
	int32_t			mn;
	int32_t			width;
	bool			ellipsis;
	t_point			at;
}	t_textrun;

typedef struct s_edit
{
	uint32_t	caret;
	uint32_t	anchor;
	int32_t		scroll;
	uint32_t	max_chars;
}	t_edit;

typedef struct s_edview
{
	const t_font	*font;
	char			disp[CTL_TEXT_MAX];
	int32_t			len;
	t_point			org;
	t_color			col;
}	t_edview;

typedef struct s_clip
{
	char		data[CTL_TEXT_MAX];
	uint32_t	len;
}	t_clip;

typedef struct s_list
{
	char		**items;
	uint32_t	count;
	uint32_t	cap;
	int32_t		sel;
	t_scrst		sb;
}	t_list;

typedef struct s_lstgeom
{
	t_rect	items;
	t_rect	bar;
	int32_t	rowh;
	int32_t	rows;
	bool	bar_on;
}	t_lstgeom;

typedef struct s_menulevel
{
	const t_ctlmenuitem	*items;
	uint32_t			count;
	int32_t				sel;
	t_rect				rect;
}	t_menulevel;

typedef struct s_menu
{
	uint32_t	depth;
	uint32_t	picked;
	t_ctl		*prev;
	t_menulevel	lv[CTL_MENU_LEVELS_MAX];
}	t_menu;

typedef struct s_pool
{
	char	*cur;
	size_t	left;
}	t_pool;

typedef struct s_progress
{
	uint32_t	pct;
	uint32_t	reserved;
}	t_progress;

typedef struct s_rootpriv
{
	t_ctlclock	clock;
	int64_t		click_ns;
	t_point		click_at;
	uintptr_t	click_ctl;
	uint32_t	buttons;
	uint32_t	reserved;
}	t_rootpriv;

typedef struct s_focusscan
{
	t_ctl	*first;
	t_ctl	*last;
	t_ctl	*before;
	t_ctl	*after;
}	t_focusscan;

typedef struct s_ctlops
{
	size_t	priv_size;
	bool	supported;
	bool	hot;
	void	(*init)(t_ctl *c);
	void	(*destroy)(t_ctl *c);
	void	(*paint)(t_ctlroot *r, t_ctl *c);
	bool	(*mouse)(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
	bool	(*key)(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
	bool	(*activate)(t_ctlroot *r, t_ctl *c);
}	t_ctlops;

extern const t_ctlops	g_ctl_ops[CT_TYPES];

void			ctl_btn_paint(t_ctlroot *r, t_ctl *c);
bool			ctl_btn_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
bool			ctl_btn_activate(t_ctlroot *r, t_ctl *c);
void			ctl_chk_paint(t_ctlroot *r, t_ctl *c);
bool			ctl_chk_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
bool			ctl_chk_activate(t_ctlroot *r, t_ctl *c);
t_color			ctl_color_text(bool enabled);
t_color			ctl_color_shadow(void);
t_color			ctl_color_selected_text(void);
t_color			ctl_color_back(bool enabled);
t_color			ctl_color_track(void);
bool			ctl_rect_ok(t_rect r);
void			ctl_dirty_add(t_ctlroot *r, t_rect rect);
t_rootpriv		*ctl_rootpriv(const t_ctlroot *r);
uint32_t		ctl_edt_char(t_ctl *c, t_edit *e, const t_inpevent *ev);
bool			ctl_edt_copy(t_ctl *c, t_edit *e);
bool			ctl_edt_paste(t_ctl *c, t_edit *e);
bool			ctl_edt_finish(t_ctlroot *r, t_ctl *c, uint32_t res);
void			ctl_edt_init(t_ctl *c);
void			ctl_edt_clamp(t_ctl *c, t_edit *e);
bool			ctl_edt_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
bool			ctl_edt_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
bool			ctl_edt_nav(t_ctl *c, t_edit *e, uint32_t code, bool shift);
void			ctl_edt_paint(t_ctlroot *r, t_ctl *c);
void			ctl_edt_scroll(t_ctl *c, t_edit *e);
bool			ctl_edt_has_sel(const t_edit *e);
uint32_t		ctl_edt_sel_from(const t_edit *e);
uint32_t		ctl_edt_sel_to(const t_edit *e);
bool			ctl_edt_erase_sel(t_ctl *c, t_edit *e);
void			ctl_edt_select_all(t_ctl *c, t_edit *e);
bool			ctl_edt_insert(t_ctl *c, t_edit *e, const char *s, uint32_t n);
bool			ctl_edt_erase_prev(t_ctl *c, t_edit *e);
bool			ctl_edt_erase_next(t_ctl *c, t_edit *e);
uint32_t		ctl_edt_dpos(const t_ctl *c, uint32_t off);
bool			ctl_edt_view(const t_ctl *c, t_edview *v);
int32_t			ctl_edt_wv(const t_edview *v, const t_ctl *c, uint32_t upto);
int32_t			ctl_edt_width(const t_ctl *c, uint32_t upto);
uint32_t		ctl_edt_index_at(const t_ctl *c, int32_t x);
t_ctl			*ctl_find_id(const t_ctlroot *r, uint32_t id);
uint32_t		ctl_depth(const t_ctl *c);
bool			ctl_in_tree(const t_ctlroot *r, const t_ctl *c);
bool			ctl_alive(const t_ctlroot *r, const t_ctl *c);
bool			ctl_tabstop(const t_ctl *c);
void			ctl_focus_scan(const t_ctlroot *r, t_focusscan *sc);
bool			ctl_focus_after(t_ctlroot *r, const t_ctl *c);
void			ctl_forget(t_ctlroot *r, t_ctl *c);
t_ctl			*ctl_hit(const t_ctlroot *r, t_point p);
bool			ctl_key_global(t_ctlroot *r, const t_inpevent *ev);
void			ctl_link_last(t_ctl *parent, t_ctl *c);
void			ctl_unlink(t_ctl *c);
void			ctl_free_one(t_ctl *c);
void			ctl_free_subtree(t_ctl *top);
void			ctl_lst_drop(t_list *l);
bool			ctl_lst_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
bool			ctl_lst_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
void			ctl_lst_paint(t_ctlroot *r, t_ctl *c);
void			ctl_lst_init(t_ctl *c);
void			ctl_lst_destroy(t_ctl *c);
t_list			*ctl_lst_of(const t_ctlroot *r, const t_ctl *list);
bool			ctl_lst_pick(t_ctlroot *r, t_ctl *c, int32_t idx);
void			ctl_lst_geom(const t_ctl *c, t_lstgeom *g);
void			ctl_lst_sync(t_ctl *c);
void			ctl_lst_show(t_ctl *c, int32_t idx);
t_rect			ctl_lst_item_rect(const t_ctl *c, int32_t idx);
int32_t			ctl_lst_index_at(const t_ctl *c, t_point p);
t_ctl			*ctl_make(const t_ctlspec *spec, size_t extra);
void			*ctl_alloc(size_t size);
void			ctl_free(void *ptr);
bool			ctl_mnemonic_fire(t_ctlroot *r, uint32_t code);
void			ctl_mnu_end(t_ctlroot *r, t_ctl *c);
void			ctl_mnu_cancel(t_ctlroot *r, t_ctl *c);
void			ctl_mnu_activate(t_ctlroot *r, t_ctl *c, uint32_t l, int32_t i);
int32_t			ctl_mnu_item_h(const t_ctlmenuitem *it, const t_lunametrics *m);
t_rect			ctl_mnu_item_rect(const t_menu *m, uint32_t lvl, uint32_t idx);
int32_t			ctl_mnu_item_at(const t_menu *m, uint32_t lvl, t_point p);
t_rect			ctl_mnu_bounds(const t_menu *m);
bool			ctl_mnu_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
void			ctl_mnu_layout(t_ctlroot *r, t_ctl *c, uint32_t lvl);
void			ctl_mnu_refresh(t_ctlroot *r, t_ctl *c);
void			ctl_mnu_select(t_ctlroot *r, t_ctl *c, uint32_t l, int32_t i);
void			ctl_mnu_open_sub(t_ctlroot *r, t_ctl *c, uint32_t l);
void			ctl_mnu_pop(t_ctlroot *r, t_ctl *c);
bool			ctl_mnu_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
int32_t			ctl_mnu_step(const t_menu *m, uint32_t l, int32_t from, int d);
uint32_t		ctl_mnu_count(const t_menu *m, uint32_t lvl, uint32_t ch);
int32_t			ctl_mnu_find(const t_menu *m, uint32_t lvl, uint32_t ch);
void			ctl_mnu_paint(t_ctlroot *r, t_ctl *c);
size_t			ctl_mnu_pool(const t_ctlmenuitem *it, uint32_t n, uint32_t d);
t_ctlmenuitem	*ctl_mnu_copy(t_pool *p, const t_ctlmenuitem *it, uint32_t n);
t_mouseev		ctl_mouse_event(t_ctlroot *r, const t_wmmouse *m);
bool			ctl_mouse_dbl(t_ctlroot *r, const t_ctl *c, const t_mouseev *e);
bool			ctl_notify(t_ctlroot *r, t_ctl *c, uint32_t code);
bool			ctl_activate(t_ctlroot *r, t_ctl *c);
void			ctl_hot_set(t_ctlroot *r, t_ctl *c);
const t_ctlops	*ctl_ops_get(uint32_t type);
void			ctl_paint_tree(t_ctlroot *r, t_ctl *c, t_rect clip);
void			ctl_pnl_paint(t_ctlroot *r, t_ctl *c);
void			ctl_lbl_paint(t_ctlroot *r, t_ctl *c);
bool			ctl_lbl_activate(t_ctlroot *r, t_ctl *c);
void			ctl_grp_paint(t_ctlroot *r, t_ctl *c);
bool			ctl_press_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
void			ctl_prg_paint(t_ctlroot *r, t_ctl *c);
void			ctl_radio_select(t_ctlroot *r, t_ctl *c);
bool			ctl_radio_stop(const t_ctl *c);
t_ctl			*ctl_radio_step(const t_ctl *c, bool forward);
bool			ctl_rad_activate(t_ctlroot *r, t_ctl *c);
bool			ctl_rad_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
void			ctl_refs_check(t_ctlroot *r);
void			ctl_reap(t_ctlroot *r);
void			ctl_focus_ring(t_surface *s, t_rect r);
void			ctl_focus_ring_row(t_surface *s, t_rect r, bool sel);
void			ctl_sb_arrow(t_surface *s, t_rect r, int dir, t_color col);
int32_t			ctl_sb_axis(bool vertical, t_point p);
int32_t			ctl_sb_start(bool vertical, t_rect r);
uint32_t		ctl_sb_zone(const t_sblo *lo, t_point p);
int32_t			ctl_sb_pos_from(const t_sblo *lo, const t_scrst *s, int32_t at);
bool			ctl_sb_set(t_scrst *st, int32_t pos);
bool			ctl_sb_vertical(t_rect r);
int32_t			ctl_sb_len(t_rect r, bool vertical);
void			ctl_sb_layout(t_rect bar, const t_scrst *st, t_sblo *lo);
int				ctl_sb_mouse(const t_sbctx *x, const t_mouseev *e);
void			ctl_sb_paint(t_surface *s, t_rect b, const t_scrst *st, bool e);
void			ctl_scr_init(t_ctl *c);
void			ctl_scr_paint(t_ctlroot *r, t_ctl *c);
bool			ctl_scr_mouse(t_ctlroot *r, t_ctl *c, const t_mouseev *e);
bool			ctl_scr_key(t_ctlroot *r, t_ctl *c, const t_inpevent *ev);
bool			ctl_focus_move(t_ctlroot *r, bool forward);
void			ctl_copy_text(char *dst, const char *src);
int32_t			ctl_text_strip(const char *text, char *out, int32_t *mn);
int				ctl_mnemonic(const char *text);
void			ctl_text_draw(t_surface *s, const t_ctltext *tx);
const t_font	*ctl_font(uint32_t flags);
const t_font	*ctl_font_of(const t_ctl *c);
t_rect			ctl_btn_face(const t_ctl *c);
void			ctl_text_init(t_ctltext *tx, const t_ctl *c);
t_lunastate		ctl_lstate(const t_ctl *c);
bool			ctl_run_init(t_textrun *run, const t_ctltext *tx);
void			ctl_run_fit(t_textrun *run, const t_ctltext *tx);
void			ctl_run_place(t_textrun *run, const t_ctltext *tx);
int32_t			ctl_text_width(const t_ctltext *tx);
uint32_t		ctl_u8_len(const char *s, uint32_t avail);
uint32_t		ctl_u8_next(const char *s, uint32_t len, uint32_t pos);
uint32_t		ctl_u8_count(const char *s, uint32_t len);
uint32_t		ctl_u8_encode(uint32_t cp, char *out);
uint32_t		ctl_u8_prev(const char *s, uint32_t pos);
t_ctl			*ctl_walk_next(const t_ctl *c);
bool			ctl_is_within(const t_ctl *c, const t_ctl *top);
bool			ctl_shown(const t_ctl *c);
bool			ctl_enabled(const t_ctl *c);
bool			ctl_usable(const t_ctl *c);
int32_t			ctl_wheel_delta(int32_t wheel);

#endif
