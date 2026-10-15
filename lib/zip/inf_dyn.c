#include "zip_int.h"

static const uint8_t	g_order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4,
	12, 3, 13, 2, 14, 1, 15};

static int	dyn_clen(t_inf *s, uint32_t hclen)
{
	uint8_t		lens[19];
	uint32_t	i;
	uint32_t	v;

	i = 0;
	while (i < 19)
		lens[i++] = 0;
	i = 0;
	while (i < hclen)
	{
		if (inf_bits(s, 3, &v) < 0)
			return (E_INVAL);
		lens[g_order[i]] = (uint8_t)v;
		i++;
	}
	if (huff_build(&s->lit, lens, 19) != 0)
		return (E_INVAL);
	return (0);
}

static int	dyn_repeat(t_inf *s, uint32_t sym, uint32_t *rep)
{
	uint32_t	v;
	uint32_t	nbits;
	uint32_t	base;

	nbits = 2;
	base = 3;
	if (sym == 17)
		nbits = 3;
	if (sym == 18)
	{
		nbits = 7;
		base = 11;
	}
	if (inf_bits(s, nbits, &v) < 0)
		return (E_INVAL);
	*rep = base + v;
	return (0);
}

static int	dyn_lens(t_inf *s, uint8_t *lens, uint32_t total)
{
	uint32_t	i;
	uint32_t	sym;
	uint32_t	rep;
	uint8_t		val;

	i = 0;
	while (i < total)
	{
		if (huff_decode(s, &s->lit, &sym) < 0)
			return (E_INVAL);
		rep = 1;
		val = (uint8_t)sym;
		if (sym >= 16 && (dyn_repeat(s, sym, &rep) < 0
				|| (sym == 16 && i == 0)))
			return (E_INVAL);
		if (sym == 16)
			val = lens[i - 1];
		if (sym > 16)
			val = 0;
		if (rep > total - i)
			return (E_INVAL);
		while (rep-- > 0)
			lens[i++] = val;
	}
	return (0);
}

int	inf_dynamic(t_inf *s)
{
	uint8_t		lens[320];
	uint32_t	hlit;
	uint32_t	hdist;
	uint32_t	hclen;

	if (inf_bits(s, 5, &hlit) < 0 || inf_bits(s, 5, &hdist) < 0
		|| inf_bits(s, 4, &hclen) < 0)
		return (E_INVAL);
	hlit += 257;
	hdist += 1;
	if (hlit > 286 || hdist > 30 || dyn_clen(s, hclen + 4) < 0)
		return (E_INVAL);
	if (dyn_lens(s, lens, hlit + hdist) < 0 || lens[256] == 0)
		return (E_INVAL);
	if (huff_build(&s->lit, lens, hlit) < 0
		|| huff_build(&s->dist, lens + hlit, hdist) < 0)
		return (E_INVAL);
	return (0);
}
