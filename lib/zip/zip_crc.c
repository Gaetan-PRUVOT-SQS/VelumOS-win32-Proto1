#include "zip_int.h"

static const uint32_t	g_crc4[16] = {0x00000000, 0x1db71064, 0x3b6e20c8,
	0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320,
	0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278,
	0xbdbdf21c};

uint32_t	zip_crc32(uint32_t crc, const uint8_t *p, size_t n)
{
	size_t	i;

	crc = ~crc;
	i = 0;
	while (i < n)
	{
		crc ^= p[i];
		crc = (crc >> 4) ^ g_crc4[crc & 15];
		crc = (crc >> 4) ^ g_crc4[crc & 15];
		i++;
	}
	return (~crc);
}

uint32_t	zip_rd16(t_span s, size_t off)
{
	return ((uint32_t)s.p[off] | ((uint32_t)s.p[off + 1] << 8));
}

uint32_t	zip_rd32(t_span s, size_t off)
{
	return (zip_rd16(s, off) | (zip_rd16(s, off + 2) << 16));
}
