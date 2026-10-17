#ifndef LUNA_INT_H
# define LUNA_INT_H

# include <stdbool.h>
# include <stdint.h>
# include "velum/font.h"
# include "velum/luna.h"

# define LG_NBTN 3
# define LG_FRAMED 0x0001
# define LG_SIZABLE 0x0002
# define LG_MAX 0x0004
# define LG_TOOL 0x0008
# define LG_SYSMENU 0x0010
# define LG_SHOWN 0x0100
# define LG_ENABLED 0x1000
# define LG_FRAMELESS_STYLES 0x0e00
# define LG_DIM_MAX 1048576
# define LG_POS_MAX 268435456

# define LP_COORD_MAX 16777216
# define LP_WALL_MAX 16384
# define LP_RMAX 32
# define LP_PMAX 6
# define LP_OPS_ARGS 12
# define LP_TEXT_MAX 4096
# define LP_ICON_MAX 512
# define LP_LETTERS 7
# define LP_LETTER_GAP 5
# define LP_BLK_GAP 3
# define LP_BLOBS 4
# define LX_SCALE 4
# define LX_PASSES 3
# define LP_VEILS 2
# define LP_ELLIPSIS "\xe2\x80\xa6"

# define LA_LEFT 0
# define LA_CENTER 1
# define LA_RIGHT 2

# define LOP_RECT 1
# define LOP_RRECT 2
# define LOP_DISC 3
# define LOP_POLY 4
# define LOP_LINE 5
# define LOP_RING 6

# define LC_BLACK 0xff000000
# define LC_WHITE 0xffffffff
# define LC_WINDOW 0xffece9d8
# define LC_SELECTION 0xff316ac5
# define LC_TEXT 0xff000000
# define LC_GRAYTEXT 0xffaca899

typedef struct s_lpal
{
	t_color	accent;
	t_color	accent_hi;
	t_color	accent_text;
	t_color	sel_bg;
	t_color	sel_ring;
	t_color	ink;
	t_color	ink_2;
	t_color	muted;
	t_color	dim;
	t_color	danger;
	t_color	frame;
	t_color	dock;
	t_color	chip;
	t_color	panel;
	t_color	window;
	t_color	content;
	t_color	control;
	t_color	sel;
	t_color	input;
	t_color	hover;
	t_color	press;
	t_color	rim_top;
	t_color	rim_in;
	t_color	rim_bottom;
	t_color	spec;
	t_color	shadow_win;
	t_color	shadow_dock;
	t_color	opaque_panel;
	t_color	opaque_content;
	t_color	wall[5];
	int32_t	sat;
	int32_t	sat_panel;
	int32_t	veil_a;
	int32_t	veil_b;
}	t_lpal;

typedef const t_lpal		*t_cpal;

typedef const t_lunametrics	*t_cmet;

typedef struct s_lunadetail
{
	int32_t	btn_gap;
	int32_t	btn_top;
	int32_t	btn_right;
	int32_t	btn_top_max;
	int32_t	btn_right_max;
	int32_t	sysmenu_w;
	int32_t	title_pad;
	int32_t	corner_len;
	int32_t	tool_caption_h;
	int32_t	tool_btn;
	int32_t	tool_btn_top;
	int32_t	tool_btn_right;
	int32_t	check_size;
	int32_t	group_line_y;
	int32_t	group_pad;
	int32_t	progress_block;
	int32_t	progress_gap;
	int32_t	menu_header_h;
	int32_t	menu_footer_h;
	int32_t	menu_foot_item_w;
	int32_t	menu_foot_icon;
	int32_t	avatar;
	int32_t	task_gap;
	int32_t	thumb_min;
	int32_t	title_radius;
	int32_t	resize;
	int32_t	icon_left;
	int32_t	caption_icon;
}	t_lunadetail;

typedef const t_lunadetail	*t_cdet;

typedef struct s_lgeo
{
	t_rect		outer;
	t_rect		caption;
	t_rect		sysmenu;
	t_rect		icon;
	t_rect		title;
	t_rect		client;
	t_rect		btn[LG_NBTN];
	int32_t		border;
	int32_t		top;
	int32_t		bottom;
	int32_t		cap_end;
	uint32_t	flags;
}	t_lgeo;

typedef struct s_lbp
{
	int32_t	size;
	int32_t	height;
	int32_t	top;
	int32_t	right;
	int32_t	left;
}	t_lbp;

typedef struct s_lstop
{
	int32_t	at;
	t_color	c;
}	t_lstop;

typedef struct s_lgrad
{
	const t_lstop	*st;
	int32_t			n;
	t_color			tint;
	uint32_t		tint_t;
}	t_lgrad;

typedef struct s_lrr
{
	t_rect			r;
	int32_t			rad[4];
	const t_lgrad	*g;
	int32_t			y0;
	t_rect			hole;
	t_rect			vis;
}	t_lrr;

typedef struct s_llayers
{
	t_rect			r;
	int32_t			rad[4];
	const t_color	*ring;
	int32_t			nring;
	const t_lgrad	*fill;
	t_rect			hole;
	int32_t			skip;
}	t_llayers;

typedef struct s_lbox
{
	t_rect			r;
	int32_t			rad;
	const t_color	*ring;
	int32_t			nring;
	const t_lgrad	*fill;
}	t_lbox;

typedef struct s_lspan
{
	int32_t	y;
	int32_t	x0;
	int32_t	x1;
	t_color	c;
}	t_lspan;

typedef struct s_ledge
{
	int32_t	x;
	int32_t	y;
	int32_t	dir;
	int32_t	rad;
	int32_t	iy;
	t_color	c;
}	t_ledge;

typedef struct s_lbc
{
	int32_t	cx;
	int32_t	cy;
	int32_t	irx;
	int32_t	iry;
	int32_t	thr;
	int32_t	ithr;
	t_color	col;
}	t_lbc;

typedef struct s_lvc
{
	int32_t	cx;
	int32_t	cy;
	int32_t	a;
	int32_t	b;
	int32_t	co;
	int32_t	si;
	int32_t	sigma;
	int32_t	peak;
	bool	down;
}	t_lvc;

typedef struct s_lwall
{
	int32_t	w;
	int32_t	h;
	int32_t	ox;
	int32_t	oy;
	t_rect	vis;
	t_lbc	bc[LP_BLOBS];
	t_lvc	vc[LP_VEILS];
	t_color	base;
}	t_lwall;

typedef struct s_lglyph
{
	const uint8_t	*pts;
	uint8_t			w;
}	t_lglyph;

typedef struct s_lmk
{
	t_point	org;
	int32_t	unit;
	t_color	c;
}	t_lmk;

typedef struct s_lboot
{
	t_rect	emblem;
	t_rect	mark;
	t_rect	bar;
	t_rect	text;
	t_rect	card;
}	t_lboot;

typedef struct s_lcring
{
	int32_t	x;
	int32_t	y;
	int32_t	dx;
	int32_t	dy;
	int32_t	rad;
	t_color	c;
}	t_lcring;

typedef struct s_lpoly
{
	t_point	p[LP_PMAX];
	int32_t	n;
}	t_lpoly;

typedef struct s_lpe
{
	int64_t	a;
	int64_t	b;
	int64_t	c;
	int64_t	inv;
}	t_lpe;

typedef struct s_lpctx
{
	t_lpe			e[LP_PMAX];
	int32_t			n;
	t_rect			vis;
	int32_t			top;
	const t_lgrad	*g;
}	t_lpctx;

typedef struct s_lg2
{
	t_lgrad	g;
	t_lstop	st[2];
}	t_lg2;

typedef struct s_ldisc
{
	int32_t	cx;
	int32_t	cy;
	int32_t	r;
	int32_t	r_in;
}	t_ldisc;

typedef struct s_lline
{
	t_point	a;
	t_point	b;
	int32_t	w;
	bool	round;
}	t_lline;

typedef struct s_lgb
{
	uint32_t	*a;
	uint32_t	*b;
	int32_t		w;
	int32_t		h;
	t_rect		src;
}	t_lgb;

typedef struct s_lgl
{
	int32_t	n;
	int32_t	step;
	int32_t	r;
}	t_lgl;

typedef struct s_lglass
{
	t_rect	r;
	t_rect	hole;
	int32_t	rad_top;
	int32_t	rad_bot;
	int32_t	blur;
	int32_t	sat;
	t_color	fill;
	t_color	opaque;
	t_color	spec;
	t_color	rim[3];
}	t_lglass;

typedef struct s_gkind
{
	t_color	fill[2];
	int32_t	blur;
	int32_t	rad;
	bool	panel;
	bool	pill;
}	t_gkind;

typedef struct s_lscale
{
	int32_t	ox;
	int32_t	oy;
	int32_t	num;
	int32_t	den;
}	t_lscale;

typedef struct s_lop
{
	uint8_t	kind;
	uint8_t	n;
	uint8_t	a[LP_OPS_ARGS];
	t_color	c0;
	t_color	c1;
}	t_lop;

typedef struct s_loplist
{
	const t_lop	*ops;
	int32_t		n;
	t_color		flat;
}	t_loplist;

typedef struct s_liconent
{
	t_iconid	id;
	const t_lop	*ops;
	int32_t		n;
}	t_liconent;

typedef const t_liconent	*t_cicon;

typedef struct s_ltext
{
	t_fontid	font;
	t_color		color;
	t_color		shadow;
	const char	*text;
	t_rect		box;
	int32_t		align;
}	t_ltext;

typedef struct s_ltx
{
	const t_font	*f;
	const char		*text;
	t_point			p;
	int32_t			n;
	int32_t			ell;
	t_color			c;
}	t_ltx;

typedef struct s_lcb
{
	t_rect		r;
	uint32_t	kind;
	t_lunastate	st;
	bool		active;
	bool		maximized;
}	t_lcb;

t_cicon		lp_icons_a(void);
t_cicon		lp_icons_b(void);
t_cicon		lp_icons_c(void);
t_cpal		lp_pal(void);
bool		lp_gscratch(size_t px, uint32_t **a, uint32_t **b);
int32_t		lp_rmask(const t_lglass *g, int32_t x, int32_t y);
void		lp_gb_down(const t_surface *s, t_rect src, t_lgb *o);
void		lp_gb_blur(t_lgb *o, int32_t r);
void		lp_gb_line(const uint32_t *in, uint32_t *out, const t_lgl *p);
t_color		lp_gb_sample(const t_lgb *o, int32_t x, int32_t y);
t_color		lp_saturate(t_color c, int32_t sat10);
bool		lp_glass_make(t_lglass *g, t_rect r, t_glasskind kind);
void		lp_glass(t_surface *s, const t_lglass *g);
void		lp_glass_rim(t_surface *s, const t_lglass *g);
void		lp_frame_glass(t_surface *s, const t_lgeo *g, bool active);
void		lp_capsule(t_surface *s, const t_lunawin *w, const t_lgeo *g);
void		lp_capbuttons(t_surface *s, const t_lunawin *w, const t_lgeo *g);
void		lp_capglyph(t_surface *s, const t_lcb *cb, t_color c);
int32_t		lp_frame_radius(const t_lgeo *g);
int32_t		lp_frame_bands(const t_lgeo *g, t_rect *out);
void		lp_wall_setup(t_lwall *c, t_rect r, const t_surface *s);
t_color		lp_wall_blobs(const t_lwall *c, int32_t x, int32_t y);
t_color		lp_wall_veils(const t_lwall *c, t_color col, int32_t x, int32_t y);
int32_t		lp_smooth256(int32_t t);
t_cmet		lm_metrics(void);
t_cdet		lm_detail(void);
void		lg_make(const t_lunawin *w, t_lgeo *g);
void		lg_buttons(const t_lunawin *w, t_lgeo *g);
void		lg_zones(t_lgeo *g);
bool		lg_inside(t_rect a, t_rect b);
uint32_t	lg_btn_ht(int32_t i);
bool		lp_ok(const t_surface *s);
bool		lp_pt_ok(t_point p);
t_rect		lp_clean(t_rect r);
t_point		lp_pt(int32_t x, int32_t y);
t_rect		lp_rect(int32_t x, int32_t y, int32_t w, int32_t h);
bool		lp_in(t_rect r, t_point p);
t_rect		lp_isect(t_rect a, t_rect b);
t_rect		lp_visible(const t_surface *s, t_rect r);
t_rect		lp_inset(t_rect r, int32_t d);
int32_t		lp_min(int32_t a, int32_t b);
int32_t		lp_max(int32_t a, int32_t b);
int32_t		lp_clamp(int32_t v, int32_t lo, int32_t hi);
int32_t		lp_floor16(int32_t v);
int32_t		lp_round16(int32_t v);
t_color		lp_mix(t_color a, t_color b, uint32_t t);
void		lp_plot(t_surface *s, t_point p, t_color c, uint32_t cov);
void		lp_fill(t_surface *s, t_rect r, t_color c);
void		lp_flat(t_lgrad *g, t_lstop *st, t_color c);
t_color		lp_grad_at(const t_lgrad *g, int32_t pos);
void		lp_vfill(t_surface *s, t_rect r, const t_lgrad *g);
void		lp_hfill(t_surface *s, t_rect r, const t_lgrad *g);
int32_t		lp_corner_cov(int32_t r, int32_t ix, int32_t iy);
void		lp_corner_ring(t_surface *s, const t_lcring *k);
void		lp_rr_paint(t_surface *s, const t_lrr *rr);
void		lp_rr_span(t_surface *s, const t_lrr *rr, t_lspan *sp);
void		lp_rr_edge(t_surface *s, const t_lrr *rr, const t_ledge *e);
void		lp_layers(t_surface *s, const t_llayers *l);
void		lp_box(t_surface *s, const t_lbox *b);
uint32_t	lp_isqrt(uint64_t v);
void		lp_disc(t_surface *s, const t_ldisc *d, const t_lgrad *g);
void		lp_poly(t_surface *s, const t_lpoly *p, const t_lgrad *g);
int64_t		lp_poly_area2(const t_lpoly *p);
t_rect		lp_poly_box(const t_lpoly *p);
void		lp_line(t_surface *s, const t_lline *l, t_color c);
void		lp_ops(t_surface *s, const t_lscale *c, const t_lop *o, int32_t n);
void		lp_ops_flat(t_surface *s, const t_lscale *c, const t_loplist *l);
void		lp_ops_list(t_surface *s, const t_lscale *c, const t_loplist *l);
int32_t		lp_op_x(const t_lscale *c, int32_t v);
int32_t		lp_op_y(const t_lscale *c, int32_t v);
int32_t		lp_op_len(const t_lscale *c, int32_t v);
void		lp_op_grad(const t_lop *op, int32_t at0, int32_t at1, t_lg2 *o);
void		lp_op_rect(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_op_rrect(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_op_disc(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_op_ring(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_op_poly(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_op_line(t_surface *s, const t_lscale *c, const t_lop *o);
void		lp_text(t_surface *s, const t_ltext *t);
int32_t		lp_text_len(const char *s);
void		lp_dotted(t_surface *s, t_rect r, t_color c);
void		lp_sthumb(t_surface *s, const t_rect *r, t_lunastate st, bool vert);
void		lp_arrow(t_surface *s, t_rect r, int32_t dir, t_color c);
t_loplist	lp_icon_list(t_iconid id);
void		lp_emblem(t_surface *s, t_rect r);
void		lp_boot_bar(t_surface *s, const t_rect *r, uint32_t p, uint32_t t);
void		lp_wordmark(t_surface *s, t_rect r, t_color c);
int32_t		lp_wordmark_w(int32_t h);

#endif
