#include <stdlib.h>
#include <sys/stat.h>
#include "lt.h"

static void	png_row(const t_surface *s, int32_t zoom, int32_t y, uint8_t *o)
{
	uint32_t	c;
	size_t		row;
	int32_t		x;

	row = (size_t)(y / zoom) * (size_t)s->stride;
	o[0] = 0;
	x = 0;
	while (x < s->w * zoom)
	{
		c = s->px[row + (size_t)(x / zoom)];
		o[1 + 3 * x] = (uint8_t)(c >> 16);
		o[2 + 3 * x] = (uint8_t)(c >> 8);
		o[3 + 3 * x] = (uint8_t)c;
		x++;
	}
}

static uint8_t	*png_raw(const t_surface *s, int32_t zoom, size_t *size)
{
	uint8_t	*raw;
	size_t	line;
	int32_t	y;

	line = 1 + (size_t)(s->w * zoom) * 3;
	*size = (size_t)(s->h * zoom) * line;
	raw = malloc(*size);
	y = 0;
	while (raw != NULL && y < s->h * zoom)
	{
		png_row(s, zoom, y, raw + (size_t)y * line);
		y++;
	}
	return (raw);
}

static void	png_block(t_png *p, const uint8_t *b, size_t n, int last)
{
	uint8_t	hdr[5];

	hdr[0] = (uint8_t)last;
	hdr[1] = (uint8_t)n;
	hdr[2] = (uint8_t)(n >> 8);
	hdr[3] = (uint8_t)(255 - hdr[1]);
	hdr[4] = (uint8_t)(255 - hdr[2]);
	png_write(p, hdr, 5);
	png_write(p, b, n);
}

static void	png_idat(t_png *p, const uint8_t *raw, size_t size)
{
	size_t		off;
	size_t		n;
	uint32_t	ad;

	n = (size + 65534) / 65535;
	png_begin(p, "IDAT", (uint32_t)(2 + n * 5 + size + 4));
	png_write(p, "\x78\x01", 2);
	ad = 1;
	off = 0;
	while (off < size)
	{
		n = size - off;
		if (n > 65535)
			n = 65535;
		png_block(p, raw + off, n, off + n == size);
		ad = lt_adler32(ad, raw + off, n);
		off += n;
	}
	png_u32(p, ad);
	png_end(p);
}

int	lt_png(const char *path, const t_surface *s, int32_t zoom)
{
	t_png	p;
	uint8_t	*raw;
	size_t	size;

	raw = png_raw(s, zoom, &size);
	p.f = fopen(path, "wb");
	if (raw == NULL || p.f == NULL)
	{
		free(raw);
		return (-1);
	}
	fwrite("\x89PNG\r\n\x1a\n", 1, 8, p.f);
	png_begin(&p, "IHDR", 13);
	png_u32(&p, (uint32_t)(s->w * zoom));
	png_u32(&p, (uint32_t)(s->h * zoom));
	png_write(&p, "\x08\x02\x00\x00\x00", 5);
	png_end(&p);
	png_idat(&p, raw, size);
	png_begin(&p, "IEND", 0);
	png_end(&p);
	fclose(p.f);
	free(raw);
	return (0);
}
