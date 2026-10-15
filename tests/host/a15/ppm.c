#include <stdio.h>
#include <string.h>
#include "ppm.h"

static char	g_dir[256] = "build/a15/host/a15";

void	visu_init(const char *argv0)
{
	const char	*slash;
	size_t		len;

	slash = strrchr(argv0, '/');
	if (slash == NULL)
		return ;
	len = (size_t)(slash - argv0);
	if (len >= sizeof(g_dir))
		return ;
	memcpy(g_dir, argv0, len);
	g_dir[len] = '\0';
}

int	visu_png(const char *name, const t_surface *s, int zoom)
{
	char	path[512];

	snprintf(path, sizeof(path), "%s/%s.png", g_dir, name);
	return (png_write(path, s, zoom));
}

static void	ppm_pixel(FILE *f, uint32_t c)
{
	fputc((int)((c >> 16) & 255), f);
	fputc((int)((c >> 8) & 255), f);
	fputc((int)(c & 255), f);
}

int	ppm_write(const char *path, const t_surface *s, int zoom)
{
	FILE	*f;
	int32_t	x;
	int32_t	y;

	f = fopen(path, "wb");
	if (f == NULL)
		return (-1);
	fprintf(f, "P6\n%d %d\n255\n", s->w * zoom, s->h * zoom);
	y = 0;
	while (y < s->h * zoom)
	{
		x = 0;
		while (x < s->w * zoom)
		{
			ppm_pixel(f, s->px[(y / zoom) * s->stride + x / zoom]);
			x++;
		}
		y++;
	}
	return (fclose(f));
}
