#include <stdlib.h>
#include "ppm.h"

uint32_t	*visu_alloc(t_surface *s, int32_t w, int32_t h)
{
	uint32_t	*px;

	px = calloc((size_t)w * (size_t)h, sizeof(uint32_t));
	if (px == NULL)
		abort();
	gfx_surface_init(s, px, w, h);
	return (px);
}

int	visu_save(const char *name, t_surface *s, int zoom)
{
	int	rc;

	rc = visu_png(name, s, zoom);
	free(s->px);
	return (rc);
}
