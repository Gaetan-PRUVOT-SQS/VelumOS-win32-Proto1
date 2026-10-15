#include <sys/stat.h>
#include "lt_gal.h"

int	lt_out_dir(void)
{
	mkdir("build", 0777);
	mkdir("build/a17", 0777);
	return (0);
}

t_surface	lt_crop(const t_surface *s, t_rect r)
{
	t_surface	c;

	c = *s;
	c.px = s->px + (size_t)r.y * (size_t)s->stride + (size_t)r.x;
	c.w = r.w;
	c.h = r.h;
	c.clip.x = 0;
	c.clip.y = 0;
	c.clip.w = r.w;
	c.clip.h = r.h;
	return (c);
}

void	lt_zoom(const t_surface *s, t_rect r, int32_t zoom, const char *path)
{
	t_surface	c;

	c = lt_crop(s, r);
	lt_png(path, &c, zoom);
}
