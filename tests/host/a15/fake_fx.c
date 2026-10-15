#include <stdlib.h>
#include <string.h>
#include "harness.h"
#include "ref.h"

static const int32_t	g_plain[4][2] = {{1, 1}, {2, 2}, {7, 5}, {33, 17}};
static const int32_t	g_inner[2][6] = {{40, 30, 3, 4, 20, 12},
{16, 10, 10, 6, 6, 4}};

static void	fx_world(t_fx *fx, int32_t ww, int32_t wh)
{
	fx->ww = ww;
	fx->wh = wh;
	fx->ox = 0;
	fx->oy = 0;
	guard_new(&fx->g, (size_t)ww * (size_t)wh);
	guard_randomize(&fx->g);
	fx->snap = malloc((size_t)ww * (size_t)wh * sizeof(uint32_t));
	if (fx->snap == NULL)
		abort();
	fx->valid = true;
	gfx_surface_init(&fx->view, fx->g.px, ww, wh);
}

static void	fx_inner(t_fx *fx, const int32_t d[6])
{
	fx_world(fx, d[0], d[1]);
	fx->ox = d[2];
	fx->oy = d[3];
	fx->view = gfx_surface_sub(&fx->view, rect_make(d[2], d[3], d[4], d[5]));
}

static void	fx_special(t_fx *fx, int kind)
{
	if (kind < 6)
		fx_inner(fx, g_inner[kind - 4]);
	else if (kind == 6)
	{
		fx_world(fx, 12, 8);
		fx->view.w = 7;
		fx->view.stride = 12;
		fx->view.clip = rect_make(0, 0, 7, 8);
	}
	else
	{
		fx_world(fx, 5, 4);
		fx->valid = false;
		fx->view.w = 8;
		fx->view.h = 4;
		fx->view.stride = 5;
		if (kind == 8)
			fx->view.px = NULL;
		if (kind == 9)
			fx->view.w = -3;
	}
}

void	fx_open(t_fx *fx, int kind)
{
	if (kind < 4)
		fx_world(fx, g_plain[kind][0], g_plain[kind][1]);
	else
		fx_special(fx, kind);
	memcpy(fx->snap, fx->g.px, (size_t)fx->ww * (size_t)fx->wh * 4);
	fx->cv = rect_make(0, 0, fx->view.w, fx->view.h);
}

void	fx_close(t_fx *fx)
{
	h_true(guard_ok(&fx->g), "zones de garde du tampon intactes");
	free(fx->snap);
	guard_free(&fx->g);
}
