#include <stdlib.h>
#include "fake_gfx.h"

t_fakegfx	g_fake;

void	fake_reset(void)
{
	g_fake.calls = 0;
	g_fake.outside = 0;
	g_fake.repeated = 0;
}

void	fake_surface(t_surface *s, uint32_t *px, int32_t w, int32_t h)
{
	int64_t	i;

	i = 0;
	while (i < (int64_t)w * h)
		px[i++] = FAKE_PAPER;
	s->px = px;
	s->w = w;
	s->h = h;
	s->stride = w;
	s->clip.x = 0;
	s->clip.y = 0;
	s->clip.w = w;
	s->clip.h = h;
}

static int	inside(const t_surface *s, t_point p)
{
	if (p.x < 0 || p.y < 0 || p.x >= s->w || p.y >= s->h)
		return (0);
	if (p.x < s->clip.x || p.y < s->clip.y)
		return (0);
	if ((int64_t)p.x >= (int64_t)s->clip.x + s->clip.w)
		return (0);
	return ((int64_t)p.y < (int64_t)s->clip.y + s->clip.h);
}

void	gfx_put(t_surface *s, t_point p, t_color c)
{
	g_fake.calls++;
	if (!inside(s, p))
	{
		g_fake.outside++;
		return ;
	}
	if (s->px[(int64_t)p.y * s->stride + p.x] != FAKE_PAPER)
		g_fake.repeated++;
	s->px[(int64_t)p.y * s->stride + p.x] = c;
}

void	fake_alloc(t_surface *s, int32_t w, int32_t h)
{
	uint32_t	*px;

	px = malloc((size_t)w * (size_t)h * sizeof(uint32_t));
	if (!px)
		abort();
	fake_surface(s, px, w, h);
	fake_reset();
}
