#include "lt.h"

uint32_t	lt_crc32(uint32_t crc, const uint8_t *b, size_t n)
{
	uint32_t	c;
	int			k;

	c = ~crc;
	while (n > 0)
	{
		c ^= *b;
		k = 0;
		while (k < 8)
		{
			if (c & 1)
				c = (c >> 1) ^ 0xedb88320u;
			else
				c >>= 1;
			k++;
		}
		b++;
		n--;
	}
	return (~c);
}

uint32_t	lt_adler32(uint32_t a, const uint8_t *b, size_t n)
{
	uint32_t	s1;
	uint32_t	s2;

	s1 = a & 0xffff;
	s2 = a >> 16;
	while (n > 0)
	{
		s1 = (s1 + *b) % 65521;
		s2 = (s2 + s1) % 65521;
		b++;
		n--;
	}
	return ((s2 << 16) | s1);
}
