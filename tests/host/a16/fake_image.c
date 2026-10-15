#include <stdio.h>
#include <stdlib.h>
#include "a16_test.h"

uint64_t	fake_hash(const t_surface *s)
{
	uint64_t	h;
	int64_t		i;

	h = 14695981039346656037ull;
	i = 0;
	while (i < (int64_t)s->w * s->h)
	{
		h = (h ^ (s->px[i] & 0xFFFFFF)) * 1099511628211ull;
		i++;
	}
	return (h);
}

static void	put_pixel(FILE *f, uint32_t px, int scale)
{
	int	k;

	k = 0;
	while (k < scale)
	{
		fputc((px >> 16) & 0xFF, f);
		fputc((px >> 8) & 0xFF, f);
		fputc(px & 0xFF, f);
		k++;
	}
}

static void	put_row(FILE *f, const uint32_t *row, int32_t w, int scale)
{
	int32_t	x;

	x = 0;
	while (x < w)
	{
		put_pixel(f, row[x], scale);
		x++;
	}
}

int	fake_write_ppm(const t_surface *s, const char *path, int scale)
{
	FILE	*f;
	int32_t	y;
	int		k;

	f = fopen(path, "wb");
	if (!f)
		return (-1);
	fprintf(f, "P6\n%d %d\n255\n", s->w * scale, s->h * scale);
	y = 0;
	while (y < s->h)
	{
		k = 0;
		while (k++ < scale)
			put_row(f, s->px + (int64_t)y * s->stride, s->w, scale);
		y++;
	}
	return (fclose(f));
}

uint64_t	render_hash(const t_textreq *rq)
{
	t_surface	s;
	uint32_t	*px;
	uint64_t	h;

	px = malloc(RENDER_W * RENDER_H * sizeof(uint32_t));
	if (!px)
		abort();
	fake_surface(&s, px, RENDER_W, RENDER_H);
	fake_reset();
	font_draw(&s, rq);
	h = fake_hash(&s);
	free(px);
	return (h);
}
