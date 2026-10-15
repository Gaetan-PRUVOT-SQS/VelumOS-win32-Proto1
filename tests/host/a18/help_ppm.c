#include <stdio.h>
#include <stdlib.h>
#include "help.h"

static int	write_rows(FILE *f, const t_surface *s)
{
	int32_t			x;
	int32_t			y;
	uint32_t		p;
	unsigned char	rgb[3];

	y = 0;
	while (y < s->h)
	{
		x = 0;
		while (x < s->w)
		{
			p = s->px[(size_t)y * (size_t)s->stride + (size_t)x];
			rgb[0] = (unsigned char)(p >> 16);
			rgb[1] = (unsigned char)(p >> 8);
			rgb[2] = (unsigned char)p;
			if (fwrite(rgb, 1, 3, f) != 3)
				return (-1);
			x++;
		}
		y++;
	}
	return (0);
}

int	hp_write(const char *name, const t_surface *s)
{
	const char	*dir;
	char		path[512];
	FILE		*f;
	int			r;

	dir = getenv("A18_OUT");
	if (dir == NULL)
		dir = "build/a18";
	snprintf(path, sizeof(path), "%s/%s.ppm", dir, name);
	f = fopen(path, "wb");
	if (f == NULL)
		return (-1);
	fprintf(f, "P6\n%d %d\n255\n", s->w, s->h);
	r = write_rows(f, s);
	if (fclose(f) != 0)
		r = -1;
	return (r);
}

uint32_t	hp_hash(const t_surface *s)
{
	uint32_t	h;
	int32_t		x;
	int32_t		y;

	h = 2166136261u;
	y = 0;
	while (y < s->h)
	{
		x = 0;
		while (x < s->w)
		{
			h = (h ^ (s->px[(size_t)y * (size_t)s->stride + (size_t)x]
						& 0xffffff)) * 16777619u;
			x++;
		}
		y++;
	}
	return (h);
}
