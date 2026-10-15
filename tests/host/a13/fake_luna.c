#include "fakes.h"
#include "velum/luna.h"

t_fluna	g_fl;

void	luna_boot_draw(t_surface *s, uint32_t pct, uint32_t tick)
{
	t_rect	bar;

	g_fl.calls++;
	g_fl.pct = pct;
	g_fl.tick = tick;
	g_fl.w = s->w;
	g_fl.h = s->h;
	gfx_fill(s, rect_make(0, 0, s->w, s->h), 0xff000000);
	bar = rect_make(s->w / 4, s->h * 3 / 4, s->w / 2 * (int32_t)pct / 100, 8);
	gfx_fill(s, bar, 0xff0058ee);
}
