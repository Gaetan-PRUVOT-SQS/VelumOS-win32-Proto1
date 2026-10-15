#include "dex_int.h"

uint32_t	dex_adler32(const uint8_t *p, size_t n)
{
	uint32_t	a;
	uint32_t	b;
	size_t		i;
	size_t		stop;

	a = 1;
	b = 0;
	i = 0;
	while (i < n)
	{
		stop = n;
		if (n - i > 5552)
			stop = i + 5552;
		while (i < stop)
		{
			a += p[i++];
			b += a;
		}
		a %= 65521;
		b %= 65521;
	}
	return ((b << 16) | a);
}
