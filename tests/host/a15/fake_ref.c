#include "ref.h"

uint32_t	ref_div255(uint64_t x)
{
	return ((uint32_t)((x + 127) / 255));
}

static uint32_t	ref_chan(uint64_t a, uint64_t s, uint64_t d)
{
	return (ref_div255(a * s + (255 - a) * d));
}

t_color	ref_blend(t_color dst, t_color src)
{
	uint32_t	a;
	uint32_t	out;
	int			ch;

	a = src >> 24;
	out = ref_chan(a, 255, dst >> 24) << 24;
	ch = 0;
	while (ch < 3)
	{
		out |= ref_chan(a, (src >> (8 * ch)) & 255, (dst >> (8 * ch)) & 255)
			<< (8 * ch);
		ch++;
	}
	return (out);
}

int64_t	ref_ramp(int64_t from, int64_t to, int64_t n, int64_t i)
{
	__int128	num;
	int64_t		den;

	den = n - 1;
	if (den <= 0)
		return (from);
	num = (__int128)from * den + (__int128)(to - from) * i;
	return ((int64_t)((2 * num + den) / (2 * (__int128)den)));
}

t_color	ref_spread(uint32_t a, uint32_t v, uint32_t mul)
{
	return ((a << 24) | (((v * 7) & 255) << 16) | (((v * mul) & 255) << 8)
		| ((v * 13) & 255));
}
