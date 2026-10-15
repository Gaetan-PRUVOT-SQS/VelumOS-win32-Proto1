#include <string.h>
#include "lt.h"

void	png_write(t_png *p, const void *b, size_t n)
{
	fwrite(b, 1, n, p->f);
	p->crc = lt_crc32(p->crc, b, n);
}

void	png_u32(t_png *p, uint32_t v)
{
	uint8_t	b[4];

	b[0] = (uint8_t)(v >> 24);
	b[1] = (uint8_t)(v >> 16);
	b[2] = (uint8_t)(v >> 8);
	b[3] = (uint8_t)v;
	png_write(p, b, 4);
}

void	png_begin(t_png *p, const char *type, uint32_t len)
{
	uint8_t	b[4];

	b[0] = (uint8_t)(len >> 24);
	b[1] = (uint8_t)(len >> 16);
	b[2] = (uint8_t)(len >> 8);
	b[3] = (uint8_t)len;
	fwrite(b, 1, 4, p->f);
	p->crc = 0;
	png_write(p, type, 4);
}

void	png_end(t_png *p)
{
	uint8_t	b[4];

	b[0] = (uint8_t)(p->crc >> 24);
	b[1] = (uint8_t)(p->crc >> 16);
	b[2] = (uint8_t)(p->crc >> 8);
	b[3] = (uint8_t)p->crc;
	fwrite(b, 1, 4, p->f);
}
