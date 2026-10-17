#ifndef A16_TEST_H
# define A16_TEST_H

# include <stddef.h>
# include <stdint.h>
# include <string.h>
# include "fake_gfx.h"
# include "fake_utf8.h"
# include "harness.h"
# include "font_int.h"

# define RENDER_W 96
# define RENDER_H 40

typedef struct s_style
{
	t_fontid	font;
	t_color		color;
}	t_style;

typedef struct s_vec
{
	const char	*bytes;
	int32_t		len;
	uint32_t	cp;
}	t_vec;

typedef enum e_cset
{
	CS_FRENCH,
	CS_DISPLAY,
	CS_DIGITS
}	t_cset;

typedef struct s_csetdef
{
	const uint32_t	*list;
	size_t			len;
	int32_t			ascii;
}	t_csetdef;

t_cset			charset_of(t_fontid id);
int				charset_has(t_cset set, uint32_t cp);
int32_t			charset_count(t_cset set);
uint64_t		rng_next(uint64_t *state);
uint64_t		rng_seed(void);
int64_t			model_width(const t_font *f, const char *s, size_t len);
size_t			model_boundary(const char *s, size_t len, size_t from);
int				model_is_boundary(const char *s, size_t len, size_t at);
uint8_t			*exact_copy(const void *src, size_t len);
uint32_t		decode_poisoned(const uint8_t *seq, size_t len, size_t *used);
int32_t			count_replacements(const void *seq, size_t len);
size_t			decode_n(const void *s, size_t len, uint32_t *out, size_t cap);
int				glyph_pixel(const t_glyph *g, int32_t col, int32_t row);
const t_font	*wide_font(void);
char			*repeat_char(char c, size_t count);
uint64_t		render_hash(const t_textreq *rq);
t_point			point_of(int32_t x, int32_t y);
void			req_set(t_textreq *rq, t_fontid id, t_point at, const char *s);
int32_t			glyph_ink(const t_glyph *g);
t_rect			rect_of(int32_t x, int32_t y, int32_t w, int32_t h);
void			fill_rect(t_surface *s, t_rect r, t_color c);
void			ellipsis(t_surface *s, t_rect box, t_fontid id, const char *t);
void			button_draw(t_surface *s, t_rect r, const char *label);
void			ink_of(const t_font *f, const char *s, t_point at, uint8_t *o);
const t_font	*bare_font(void);

#endif
