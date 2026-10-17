#ifndef WS_CORE_H
# define WS_CORE_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/abi/abi_types.h"
# include "velum/gfx.h"
# include "velum/luna.h"
# include "velum/wm.h"

# define WS_WIN_MAX 128
# define WS_BAND_DESKTOP 0
# define WS_BAND_NORMAL 1
# define WS_BAND_TOP 2
# define WS_DIRTY_MAX 32
# define WS_COORD_MAX 16384
# define WS_DIM_MAX 8192
# define WS_MINTRACK_W 112
# define WS_DRAG_KEEP 40
# define WS_STYLE_KNOWN 0x0fff
# define WS_CURSOR_W 16
# define WS_CURSOR_H 20
# define WS_DESK_COLOR 0xff004e98

typedef struct s_wwin
{
	uint32_t	id;
	int32_t		owner;
	uint32_t	style;
	uint32_t	state;
	uint32_t	icon;
	uint32_t	cursor;
	uint32_t	hot;
	uint32_t	pressed;
	t_rect		rect;
	t_rect		normal;
	t_surface	content;
	t_handle	section;
	uint32_t	before_min;
	uint64_t	sec_bytes;
	char		title[WM_TITLE_MAX];
}	t_wwin;

typedef struct s_wtable
{
	t_wwin		w[WS_WIN_MAX];
	uint16_t	z[WS_WIN_MAX];
	uint32_t	nz;
	uint32_t	nused;
	int32_t		active;
	uint32_t	gen;
	t_rect		screen;
	t_rect		work;
}	t_wtable;

typedef struct s_wdirty
{
	t_rect		r[WS_DIRTY_MAX];
	uint32_t	n;
	uint32_t	overflow;
	t_rect		screen;
}	t_wdirty;

typedef struct s_wframestat
{
	uint32_t	rects;
	uint32_t	overflow;
	uint64_t	px_dirty;
	uint64_t	px_comp;
	uint64_t	px_paint;
	uint64_t	px_glass;
	uint64_t	ns;
}	t_wframestat;

typedef struct s_wmeter
{
	t_wframestat	last;
	t_wframestat	total;
	t_wframestat	window;
	uint64_t		ns_max;
	uint64_t		log_at;
	uint32_t		frames;
	uint32_t		window_frames;
}	t_wmeter;

typedef struct s_wdrag
{
	int32_t		slot;
	uint32_t	ht;
	t_point		start;
	t_rect		orig;
}	t_wdrag;

typedef struct s_wcursor
{
	t_point		pos;
	uint32_t	shape;
	bool		visible;
}	t_wcursor;

typedef struct s_wcurdef
{
	const char	*pix;
	int32_t		w;
	int32_t		h;
	int32_t		hx;
	int32_t		hy;
}	t_wcurdef;

void		wt_init(t_wtable *t, t_rect screen);
int			wt_alloc(t_wtable *t, int32_t owner);
int			wt_find(const t_wtable *t, uint32_t id);
uint32_t	wt_count_owner(const t_wtable *t, int32_t owner);
void		wt_free(t_wtable *t, int slot);
uint32_t	wz_band(uint32_t style);
int			wz_index(const t_wtable *t, int slot);
void		wz_remove(t_wtable *t, int slot);
void		wz_insert(t_wtable *t, int slot);
bool		wz_raise(t_wtable *t, int slot);
bool		wf_visible(const t_wwin *w);
bool		wf_activatable(const t_wwin *w);
int			wf_next(const t_wtable *t, int exclude);
int			wz_check(const t_wtable *t);
bool		wh_decorated(uint32_t style);
void		wh_lunawin(const t_wtable *t, int slot, t_lunawin *lw);
t_rect		wh_client(const t_wtable *t, int slot);
int			wh_window_at(const t_wtable *t, t_point p);
uint32_t	wh_hit(const t_wtable *t, t_point p, int *slot);
void		wg_min_size(uint32_t style, int32_t *minw, int32_t *minh);
t_rect		wg_normalize(const t_wtable *t, uint32_t style, t_rect r);
t_rect		wg_clamp_pos(const t_wtable *t, uint32_t style, t_rect r);
t_rect		wg_maximized(const t_wtable *t);
int32_t		wg_clamp(int32_t v, int32_t lo, int32_t hi);
void		wd_begin(t_wdrag *d, int slot, uint32_t ht, t_point p);
bool		wd_is_resize(uint32_t ht);
t_rect		wd_target(const t_wdrag *d, const t_wtable *t, t_point p);
uint32_t	wd_cursor(uint32_t ht);
void		wdy_init(t_wdirty *d, t_rect screen);
void		wdy_add(t_wdirty *d, t_rect r);
t_rect		wdy_bounds(const t_wdirty *d);
bool		wr_inside(t_rect outer, t_rect inner);
t_rect		wr_offset(t_rect r, int32_t dx, int32_t dy);
uint32_t	wsp_size_of(uint32_t type);
bool		wsp_rect_ok(t_rect r);
bool		wsp_title_ok(const char *t, uint32_t max);
bool		wsp_utf8_ok(const uint8_t *s, uint32_t n);
int			wsp_validate(const void *buf, uint32_t len, uint32_t nhandles);
void		wcur_get(uint32_t shape, t_wcurdef *out);
t_rect		wcur_rect(t_wcursor c);
void		wcur_draw(t_surface *s, t_wcursor c);
void		wc_compose(const t_wtable *t, t_surface *s, t_wcursor c, t_rect d);
void		wc_window(const t_wtable *t, int slot, t_surface *s);
int32_t		wgl_bands(const t_wtable *t, int slot, t_rect *out);
t_rect		wgl_extend_n(const t_wtable *t, t_rect r, uint32_t passes);
t_rect		wgl_extend(const t_wtable *t, t_rect r);
uint64_t	wst_area(t_rect r);
uint64_t	wst_glass_px(const t_wtable *t, t_rect r);
uint64_t	wst_paint_px(const t_wtable *t, t_rect r);
void		wst_note(t_wframestat *f, const t_wtable *t, t_rect dirty,
				t_rect comp);
void		wst_close(t_wmeter *m, const t_wframestat *f);
bool		wst_due(t_wmeter *m, uint64_t now, uint64_t period);
void		wst_rearm(t_wmeter *m, uint64_t now);
uint32_t	wst_format(const t_wmeter *m, char *buf, uint32_t n);

#endif
