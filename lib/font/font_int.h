#ifndef FONT_INT_H
# define FONT_INT_H

# include <stddef.h>
# include <stdint.h>
# include "velum/font.h"

# define FONT_REPLACEMENT 0xFFFD
# define FONT_SPACE 0x20
# define FONT_NBSP 0xA0
# define FONT_NNBSP 0x202F
# define FONT_ASCII_FIRST 0x20
# define FONT_ASCII_LAST 0x7E
# define FONT_XOFF_MIN 128

typedef struct s_box
{
	int32_t	left;
	int32_t	top;
	int32_t	right;
	int32_t	bottom;
}	t_box;

typedef struct s_pen
{
	t_surface		*dst;
	const t_font	*font;
	t_color			color;
	int64_t			x;
	int64_t			y;
	t_box			box;
}	t_pen;

typedef struct s_cut
{
	const uint8_t	*rows;
	int32_t			x;
	int32_t			y;
	int32_t			c0;
	int32_t			c1;
	int32_t			r0;
	int32_t			r1;
}	t_cut;

extern const t_font	g_font_ui;
extern const t_font	g_font_ui_bold;
extern const t_font	g_font_title;
extern const t_font	g_font_mono;
extern const t_font	g_font_ins_11;
extern const t_font	g_font_ins_12;
extern const t_font	g_font_ins_semi_12;
extern const t_font	g_font_ins_13;
extern const t_font	g_font_ins_semi_13;
extern const t_font	g_font_ins_bold_13;
extern const t_font	g_font_ins_14;
extern const t_font	g_font_ins_semi_14;
extern const t_font	g_font_ins_bold_14;
extern const t_font	g_font_ins_15;
extern const t_font	g_font_ins_22;
extern const t_font	g_font_ins_semi_22;
extern const t_font	g_font_ins_bold_24;
extern const t_font	g_font_ins_bold_26;
extern const t_font	g_font_ins_bold_28;
extern const t_font	g_font_ins_semi_120;
extern const t_font	g_font_jbm_14;

const char	*font_text_end(const char *text, int32_t len);
int			font_pen_start(t_pen *pen, t_surface *dst, const t_textreq *rq);
int			font_glyph_cut(const t_pen *pen, const t_glyph *g, t_cut *cut);
int			font_glyph_rows(const t_font *f, const t_glyph *g,
				const uint8_t **rows);

#endif
