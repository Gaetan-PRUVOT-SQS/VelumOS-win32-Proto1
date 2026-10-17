#include "help.h"

static uint32_t	*g_scratch;
static size_t	g_scratch_px;

void	luna_glass_scratch(uint32_t *buf, size_t px)
{
	g_scratch = buf;
	g_scratch_px = px;
	if (buf == NULL)
		g_scratch_px = 0;
}

size_t	luna_glass_need(t_rect r)
{
	if (r.w <= 0 || r.h <= 0)
		return (0);
	return (2 * (size_t)((r.w + 3) / 4) * (size_t)((r.h + 3) / 4));
}

int32_t	luna_window_glass_bands(const t_lunawin *w, t_rect *out)
{
	if (w == NULL || out == NULL || w->outer.w <= 0 || w->outer.h <= 0)
		return (0);
	out[0] = rect_make(w->outer.x, w->outer.y, w->outer.w, 30);
	return (1);
}

int32_t	luna_client_band(const t_lunawin *w)
{
	(void)w;
	return (0);
}

void	luna_client_band_paint(t_surface *s, const t_lunawin *w,
			const t_surface *src, t_color fallback)
{
	(void)s;
	(void)w;
	(void)src;
	(void)fallback;
}
