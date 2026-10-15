#include "dex_int.h"

uint32_t	dex_uleb(t_dexcur *c)
{
	uint32_t	v;
	uint32_t	b;
	uint32_t	shift;

	v = 0;
	shift = 0;
	while (shift < 35)
	{
		b = dex_byte(c);
		if (c->err || (shift == 28 && b > 0x0f))
		{
			c->err = 1;
			return (0);
		}
		v |= (b & 0x7f) << shift;
		if (!(b & 0x80))
			return (v);
		shift += 7;
	}
	c->err = 1;
	return (0);
}

int32_t	dex_sleb(t_dexcur *c)
{
	uint32_t	v;
	uint32_t	b;
	uint32_t	shift;

	v = 0;
	shift = 0;
	b = 0x80;
	while (shift < 35 && (b & 0x80))
	{
		b = dex_byte(c);
		if (c->err || (shift == 28 && (b & 0x80)))
		{
			c->err = 1;
			return (0);
		}
		v |= (b & 0x7f) << shift;
		shift += 7;
	}
	if (shift < 32 && (b & 0x40))
		v |= ~(uint32_t)0 << shift;
	return ((int32_t)v);
}

uint32_t	dex_ulebp1(t_dexcur *c)
{
	return (dex_uleb(c) - 1);
}
