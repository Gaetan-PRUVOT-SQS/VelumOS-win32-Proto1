#include "ppm.h"

static uint32_t	adler32(const uint8_t *p, size_t n)
{
	uint32_t	a;
	uint32_t	b;

	a = 1;
	b = 0;
	while (n > 0)
	{
		a = (a + *p) % 65521;
		b = (b + a) % 65521;
		p++;
		n--;
	}
	return ((b << 16) | a);
}

static size_t	put_block(uint8_t *out, const uint8_t *raw, size_t n, int last)
{
	out[0] = (uint8_t)last;
	out[1] = (uint8_t)(n & 255);
	out[2] = (uint8_t)(n >> 8);
	out[3] = (uint8_t)(~n & 255);
	out[4] = (uint8_t)((~n >> 8) & 255);
	memcpy(out + 5, raw, n);
	return (n + 5);
}

static void	put_adler(uint8_t *out, uint32_t ad)
{
	out[0] = (uint8_t)(ad >> 24);
	out[1] = (uint8_t)(ad >> 16);
	out[2] = (uint8_t)(ad >> 8);
	out[3] = (uint8_t)ad;
}

size_t	zlib_stored(uint8_t *out, const uint8_t *raw, size_t len)
{
	size_t	pos;
	size_t	n;
	size_t	chunk;
	int		last;

	out[0] = 0x78;
	out[1] = 0x01;
	pos = 2;
	n = 0;
	last = 0;
	while (!last)
	{
		chunk = len - n;
		if (chunk > 65535)
			chunk = 65535;
		last = (n + chunk >= len);
		pos += put_block(out + pos, raw + n, chunk, last);
		n += chunk;
	}
	put_adler(out + pos, adler32(raw, len));
	return (pos + 4);
}
