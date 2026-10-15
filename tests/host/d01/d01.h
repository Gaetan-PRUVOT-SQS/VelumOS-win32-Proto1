#ifndef D01_H
# define D01_H

# include <stddef.h>
# include <stdint.h>
# include <velum/apk/zip.h>
# include <velum/err.h>

typedef struct s_blob
{
	uint8_t	*p;
	size_t	len;
}	t_blob;

typedef struct s_rec
{
	int32_t			code;
	const uint8_t	*a;
	size_t			alen;
	const uint8_t	*b;
	size_t			blen;
}	t_rec;

t_blob		d01_load(const char *name);
uint32_t	d01_u32(const uint8_t *p);
uint8_t		*d01_dup(const uint8_t *p, size_t n);
int			d01_next(const t_blob *pack, size_t *off, t_rec *r);
uint32_t	d01_rand(uint32_t *state);
int			d01_same(const uint8_t *a, const uint8_t *b, size_t n);
t_span		d01_span(const uint8_t *p, size_t n);
void		d01_mutate(uint8_t *p, size_t n, uint32_t *seed);
int			d01_walk(const t_zip *z);

#endif
