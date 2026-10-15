#ifndef KCON_INT_H
# define KCON_INT_H

# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>
# include "velum/font.h"
# include "velum/gfx.h"

# define KCON_CELL_MAX 64
# define KCON_TAB 8
# define KCON_FG 0xffc0c0c0
# define KCON_BG 0xff000000
# define KCON_INVERT 0x00ffffff
# define KCON_OPAQUE 0xff000000

# define BSOD_BLUE 0xff0000aa
# define BSOD_WHITE 0xffffffff
# define BSOD_LINES_MAX 12
# define BSOD_MARGIN 2
# define BSOD_MIN_COLS 16
# define BSOD_MIN_ROWS 8
# define BSOD_DOTS 3
# define BSOD_TAIL_ROWS 6

typedef struct s_kcon
{
	t_surface		surf;
	const t_font	*font;
	int32_t			cell_w;
	int32_t			cell_h;
	int32_t			cols;
	int32_t			rows;
	int32_t			col;
	int32_t			row;
	int32_t			cursor_x;
	int32_t			cursor_y;
	t_color			fg;
	t_color			bg;
	bool			cursor_on;
	bool			ready;
}	t_kcon;

typedef struct s_bsod_line
{
	const char	*s;
	int32_t		len;
}	t_bsod_line;

typedef struct s_bsod_wrap
{
	const char	*text;
	int32_t		cols;
	int32_t		max;
	int32_t		count;
	bool		truncated;
	t_bsod_line	lines[BSOD_LINES_MAX];
}	t_bsod_wrap;

typedef struct s_bsodtext
{
	const char	*intro;
	const char	*msg_label;
	const char	*code_label;
	const char	*build_label;
	const char	*halted;
	const char	*dots;
}	t_bsodtext;

typedef struct s_bsod
{
	t_surface		*surf;
	const t_font	*font;
	int32_t			cell_w;
	int32_t			cell_h;
	int32_t			cols;
	int32_t			rows;
	int32_t			width;
	int32_t			row;
}	t_bsod;

int			kcon_setup(t_kcon *k, const t_surface *s, const t_font *f);
void		kcon_cell_draw(t_kcon *k, const char *text, int32_t len);
void		kcon_scroll(t_kcon *k);
void		kcon_clear_all(t_kcon *k);
void		kcon_cursor_show(t_kcon *k);
void		kcon_cursor_hide(t_kcon *k);
void		kcon_newline(t_kcon *k);
void		kcon_tab(t_kcon *k);
void		kcon_backspace(t_kcon *k);
void		kcon_put(t_kcon *k, const char *text, int32_t len, uint32_t cp);
void		kcon_feed(t_kcon *k, const char *s, size_t n);
void		kcon_attach(const t_surface *s);
int32_t		bsod_cut(const char *s, int32_t len, int32_t cells);
void		bsod_wrap(t_bsod_wrap *w);
int			bsod_setup(t_bsod *b, t_surface *s, const t_font *f);
void		bsod_paint(t_bsod *b, const char *title, const char *msg);
void		bsod_text(t_bsod *b, int32_t x, const t_bsod_line *ln, t_color c);
void		bsod_title(t_bsod *b, const char *title);
void		bsod_para(t_bsod *b, const char *text, int32_t max);
void		bsod_code(t_bsod *b, const char *msg);
uint32_t	bsod_hash(const char *msg);

extern t_kcon			g_kcon;
extern const t_bsodtext	g_bsod_text;

#endif
