#ifndef LT_H
# define LT_H

# include <stddef.h>
# include <stdint.h>
# include <stdio.h>
# include "velum/luna.h"

# define LT_GUARD 8
# define LT_SENTINEL 0xdeadbeef

typedef struct s_lt
{
	t_surface	s;
	uint32_t	*buf;
	size_t		count;
	int32_t		w;
	int32_t		h;
}	t_lt;

typedef struct s_png
{
	FILE		*f;
	uint32_t	crc;
}	t_png;

int			lt_open(t_lt *t, int32_t w, int32_t h);
void		lt_close(t_lt *t);
void		lt_clear(t_lt *t, t_color c);
int			lt_guard_ok(const t_lt *t);
uint64_t	lt_hash(const t_surface *s);
uint32_t	lt_crc32(uint32_t crc, const uint8_t *b, size_t n);
uint32_t	lt_adler32(uint32_t a, const uint8_t *b, size_t n);
void		png_write(t_png *p, const void *b, size_t n);
void		png_u32(t_png *p, uint32_t v);
void		png_begin(t_png *p, const char *type, uint32_t len);
void		png_end(t_png *p);
int			lt_png(const char *path, const t_surface *s, int32_t zoom);
int			lt_out_dir(void);

#endif
