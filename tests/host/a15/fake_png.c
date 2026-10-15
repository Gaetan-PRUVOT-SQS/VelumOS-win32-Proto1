#include <stdio.h>
#include <stdlib.h>
#include "ppm.h"

static uint32_t	crc32_of(const uint8_t *p, size_t n, uint32_t crc)
{
	int	bit;

	while (n > 0)
	{
		crc ^= *p;
		bit = 0;
		while (bit < 8)
		{
			crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
			bit++;
		}
		p++;
		n--;
	}
	return (crc);
}

static void	put_chunk(FILE *f, const char *type, const uint8_t *d, size_t n)
{
	uint8_t		be[4];
	uint32_t	crc;

	be[0] = (uint8_t)(n >> 24);
	be[1] = (uint8_t)(n >> 16);
	be[2] = (uint8_t)(n >> 8);
	be[3] = (uint8_t)n;
	fwrite(be, 1, 4, f);
	fwrite(type, 1, 4, f);
	fwrite(d, 1, n, f);
	crc = crc32_of((const uint8_t *)type, 4, 0xffffffffu);
	crc = ~crc32_of(d, n, crc);
	be[0] = (uint8_t)(crc >> 24);
	be[1] = (uint8_t)(crc >> 16);
	be[2] = (uint8_t)(crc >> 8);
	be[3] = (uint8_t)crc;
	fwrite(be, 1, 4, f);
}

static size_t	fill_raw(uint8_t *raw, const t_surface *s, int zoom)
{
	size_t		pos;
	int32_t		x;
	int32_t		y;
	uint32_t	c;

	pos = 0;
	y = 0;
	while (y < s->h * zoom)
	{
		raw[pos++] = 0;
		x = 0;
		while (x < s->w * zoom)
		{
			c = s->px[(y / zoom) * s->stride + x / zoom];
			raw[pos++] = (uint8_t)(c >> 16);
			raw[pos++] = (uint8_t)(c >> 8);
			raw[pos++] = (uint8_t)c;
			x++;
		}
		y++;
	}
	return (pos);
}

static void	put_header(FILE *f, const t_surface *s, int zoom)
{
	uint8_t		ihdr[13];
	uint32_t	w;
	uint32_t	h;

	w = (uint32_t)(s->w * zoom);
	h = (uint32_t)(s->h * zoom);
	ihdr[0] = (uint8_t)(w >> 24);
	ihdr[1] = (uint8_t)(w >> 16);
	ihdr[2] = (uint8_t)(w >> 8);
	ihdr[3] = (uint8_t)w;
	ihdr[4] = (uint8_t)(h >> 24);
	ihdr[5] = (uint8_t)(h >> 16);
	ihdr[6] = (uint8_t)(h >> 8);
	ihdr[7] = (uint8_t)h;
	ihdr[8] = 8;
	ihdr[9] = 2;
	ihdr[10] = 0;
	ihdr[11] = 0;
	ihdr[12] = 0;
	fwrite("\x89PNG\r\n\x1a\n", 1, 8, f);
	put_chunk(f, "IHDR", ihdr, 13);
}

int	png_write(const char *path, const t_surface *s, int zoom)
{
	FILE	*f;
	uint8_t	*raw;
	uint8_t	*z;
	size_t	len;

	len = (size_t)(s->h * zoom) * (size_t)(1 + 3 * s->w * zoom);
	raw = malloc(len);
	z = malloc(len * 2 + 64);
	f = fopen(path, "wb");
	if (f == NULL || raw == NULL || z == NULL)
	{
		free(raw);
		free(z);
		if (f != NULL)
			fclose(f);
		return (-1);
	}
	len = fill_raw(raw, s, zoom);
	put_header(f, s, zoom);
	put_chunk(f, "IDAT", z, zlib_stored(z, raw, len));
	put_chunk(f, "IEND", z, 0);
	free(raw);
	free(z);
	return (fclose(f));
}
