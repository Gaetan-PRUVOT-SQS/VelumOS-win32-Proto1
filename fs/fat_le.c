#include "fat.h"

uint32_t	le16(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8));
}

uint32_t	le32(const uint8_t *p)
{
	return ((uint32_t)p[0] | ((uint32_t)p[1] << 8)
		| ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

void	put16(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
}

void	put32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)v;
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

int	fat_ok(const t_fat *fs, uint32_t c)
{
	return (c >= 2 && c <= fs->nclus + 1);
}
