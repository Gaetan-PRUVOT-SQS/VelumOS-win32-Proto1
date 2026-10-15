#include <stdio.h>
#include <string.h>
#include "harness.h"
#include "ref.h"

bool	fx_allowed(const t_fx *fx, int64_t wx, int64_t wy)
{
	int64_t	vx;
	int64_t	vy;

	if (!fx->valid)
		return (false);
	vx = wx - fx->ox;
	vy = wy - fx->oy;
	if (vx < 0 || vy < 0 || vx >= fx->view.w || vy >= fx->view.h)
		return (false);
	return (in_rect(fx->cv, vx, vy));
}

void	fx_clip(t_fx *fx)
{
	fx->cv = rng_rect(fx->view.w);
	if (rng_next() % 3 == 0)
		fx->cv = rect_make(0, 0, fx->view.w, fx->view.h);
	gfx_set_clip(&fx->view, fx->cv);
}

void	fx_verify(t_fx *fx, const char *what)
{
	int32_t	x;
	int32_t	y;
	size_t	i;

	y = 0;
	while (y < fx->wh)
	{
		x = 0;
		while (x < fx->ww)
		{
			i = (size_t)y * (size_t)fx->ww + (size_t)x;
			if (!fx_allowed(fx, x, y) && fx->g.px[i] != fx->snap[i])
			{
				h_true(0, what);
				fprintf(stderr, "  ecriture hors clip en (%d,%d)\n", x, y);
				return ;
			}
			x++;
		}
		y++;
	}
	h_true(guard_ok(&fx->g), what);
	memcpy(fx->snap, fx->g.px, (size_t)fx->ww * (size_t)fx->wh * 4);
}
