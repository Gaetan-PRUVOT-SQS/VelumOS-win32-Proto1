#ifndef FONT_H
# define FONT_H

# include <stdint.h>
# include "gfx.h"

typedef enum e_fontid
{
	FONT_UI = 0,
	FONT_UI_BOLD,
	FONT_TITLE,
	FONT_MONO,
	FONT_INS_11,
	FONT_INS_12,
	FONT_INS_SEMI_12,
	FONT_INS_13,
	FONT_INS_SEMI_13,
	FONT_INS_BOLD_13,
	FONT_INS_14,
	FONT_INS_SEMI_14,
	FONT_INS_BOLD_14,
	FONT_INS_15,
	FONT_INS_22,
	FONT_INS_SEMI_22,
	FONT_INS_BOLD_24,
	FONT_INS_BOLD_26,
	FONT_INS_BOLD_28,
	FONT_INS_SEMI_120,
	FONT_JBM_14,
	FONT_IDS
}	t_fontid;

typedef struct s_glyph
{
	uint32_t	cp;
	uint8_t		w;
	uint8_t		h;
	int8_t		xoff;
	int8_t		yoff;
	uint8_t		advance;
	uint16_t	off;
}	t_glyph;

typedef struct s_font
{
	int32_t			ascent;
	int32_t			descent;
	int32_t			height;
	int32_t			nglyphs;
	const t_glyph	*glyphs;
	const uint8_t	*bits;
	uint32_t		nbits;
}	t_font;

typedef struct s_textreq
{
	const t_font	*font;
	t_point			at;
	t_color			color;
	const char		*text;
	int32_t			len;
}	t_textreq;

const t_font	*font_get(t_fontid id);
const t_glyph	*font_glyph(const t_font *f, uint32_t cp);
uint32_t		font_utf8_next(const char **s, const char *end);
int32_t			font_text_width(const t_font *f, const char *text, int32_t len);
void			font_draw(t_surface *dst, const t_textreq *rq);
int32_t			font_fit(const t_font *f, const char *text, int32_t max_w);
int				font_selftest(void);

#endif
