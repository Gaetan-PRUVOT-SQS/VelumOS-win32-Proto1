#ifndef ZIP_INT_H
# define ZIP_INT_H

# include <velum/apk/zip.h>
# include <velum/err.h>

typedef struct s_huff
{
	uint16_t	count[16];
	uint16_t	sym[288];
	uint32_t	max;
}	t_huff;

typedef struct s_inf
{
	t_span		in;
	size_t		pos;
	uint32_t	acc;
	uint32_t	nb;
	uint8_t		*out;
	size_t		cap;
	size_t		n;
	t_huff		lit;
	t_huff		dist;
}	t_inf;

uint32_t	zip_rd16(t_span s, size_t off);
uint32_t	zip_rd32(t_span s, size_t off);
int			inf_bits(t_inf *s, uint32_t n, uint32_t *v);
int			inf_stored(t_inf *s);
int			huff_build(t_huff *h, const uint8_t *lens, uint32_t n);
int			huff_decode(t_inf *s, const t_huff *h, uint32_t *sym);
int			inf_fixed(t_inf *s);
int			inf_dynamic(t_inf *s);
int			inf_codes(t_inf *s);
int			zip_cd_read(const t_zip *z, uint32_t *off, t_zipent *out);

#endif
