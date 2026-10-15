#include "zip_int.h"

static void	huff_count(t_huff *h, const uint8_t *lens, uint32_t n)
{
	uint32_t	i;

	i = 0;
	while (i < 16)
		h->count[i++] = 0;
	h->max = 0;
	i = 0;
	while (i < n)
	{
		h->count[lens[i]]++;
		if (lens[i] > h->max)
			h->max = lens[i];
		i++;
	}
	h->count[0] = 0;
}

static void	huff_fill(t_huff *h, const uint8_t *lens, uint32_t n)
{
	uint16_t	offs[16];
	uint32_t	i;

	offs[1] = 0;
	i = 1;
	while (i < 15)
	{
		offs[i + 1] = (uint16_t)(offs[i] + h->count[i]);
		i++;
	}
	i = 0;
	while (i < n)
	{
		if (lens[i] != 0)
			h->sym[offs[lens[i]]++] = (uint16_t)i;
		i++;
	}
}

int	huff_build(t_huff *h, const uint8_t *lens, uint32_t n)
{
	int32_t		left;
	uint32_t	len;

	huff_count(h, lens, n);
	left = 1;
	len = 1;
	while (len <= 15)
	{
		left = left * 2 - (int32_t)h->count[len];
		if (left < 0)
			return (E_INVAL);
		len++;
	}
	if (left > 0 && h->max > 1)
		return (E_INVAL);
	huff_fill(h, lens, n);
	return (left > 0);
}

int	huff_decode(t_inf *s, const t_huff *h, uint32_t *sym)
{
	uint32_t	code;
	uint32_t	first;
	uint32_t	index;
	uint32_t	len;
	uint32_t	bit;

	code = 0;
	first = 0;
	index = 0;
	len = 0;
	while (++len <= h->max)
	{
		if (inf_bits(s, 1, &bit) < 0)
			return (E_INVAL);
		code |= bit;
		if (code - first < h->count[len])
		{
			*sym = h->sym[index + (code - first)];
			return (0);
		}
		index += h->count[len];
		first = (first + h->count[len]) << 1;
		code <<= 1;
	}
	return (E_INVAL);
}
