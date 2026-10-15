#include "blk_int.h"

#define CRC32_POLY 0xedb88320u

uint32_t	crc32_step(uint32_t state, const void *buf, size_t n)
{
	const uint8_t	*p;
	size_t			i;
	int				bit;

	p = buf;
	i = 0;
	while (i < n)
	{
		state ^= p[i];
		bit = 0;
		while (bit < 8)
		{
			state = (state >> 1) ^ (CRC32_POLY & (0u - (state & 1u)));
			bit++;
		}
		i++;
	}
	return (state);
}

uint32_t	crc32_calc(const void *buf, size_t n)
{
	return (~crc32_step(CRC32_INIT, buf, n));
}
