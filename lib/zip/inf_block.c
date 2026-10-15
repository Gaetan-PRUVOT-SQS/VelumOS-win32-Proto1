#include "zip_int.h"

static const uint16_t	g_lbase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17,
	19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const uint16_t	g_dbase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49,
	65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
	8193, 12289, 16385, 24577};

int	inf_fixed(t_inf *s)
{
	uint8_t		lens[288];
	uint32_t	i;

	i = 0;
	while (i < 288)
	{
		lens[i] = 8;
		if (i >= 144 && i < 256)
			lens[i] = 9;
		if (i >= 256 && i < 280)
			lens[i] = 7;
		i++;
	}
	huff_build(&s->lit, lens, 288);
	i = 0;
	while (i < 32)
		lens[i++] = 5;
	huff_build(&s->dist, lens, 32);
	return (0);
}

static int	inf_length(t_inf *s, uint32_t sym, uint32_t *len)
{
	uint32_t	ext;
	uint32_t	v;

	if (sym > 285)
		return (E_INVAL);
	sym -= 257;
	ext = 0;
	if (sym >= 8 && sym < 28)
		ext = (sym >> 2) - 1;
	if (inf_bits(s, ext, &v) < 0)
		return (E_INVAL);
	*len = g_lbase[sym] + v;
	return (0);
}

static int	inf_dist(t_inf *s, uint32_t *dist)
{
	uint32_t	sym;
	uint32_t	ext;
	uint32_t	v;

	if (huff_decode(s, &s->dist, &sym) < 0 || sym > 29)
		return (E_INVAL);
	ext = 0;
	if (sym >= 4)
		ext = (sym >> 1) - 1;
	if (inf_bits(s, ext, &v) < 0)
		return (E_INVAL);
	*dist = g_dbase[sym] + v;
	return (0);
}

static int	inf_copy(t_inf *s, uint32_t len, uint32_t dist)
{
	if (dist > s->n)
		return (E_INVAL);
	if (s->cap - s->n < len)
		return (E_OVERFLOW);
	while (len-- > 0)
	{
		s->out[s->n] = s->out[s->n - dist];
		s->n++;
	}
	return (0);
}

int	inf_codes(t_inf *s)
{
	uint32_t	sym;
	uint32_t	len;
	uint32_t	dist;
	int			r;

	sym = 0;
	while (sym != 256)
	{
		if (huff_decode(s, &s->lit, &sym) < 0)
			return (E_INVAL);
		if (sym < 256 && s->n >= s->cap)
			return (E_OVERFLOW);
		if (sym < 256)
			s->out[s->n++] = (uint8_t)sym;
		if (sym > 256)
		{
			if (inf_length(s, sym, &len) < 0 || inf_dist(s, &dist) < 0)
				return (E_INVAL);
			r = inf_copy(s, len, dist);
			if (r < 0)
				return (r);
		}
	}
	return (0);
}
