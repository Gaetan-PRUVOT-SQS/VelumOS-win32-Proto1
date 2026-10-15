#include <stdio.h>
#include "harness.h"
#include "ref.h"

void	scale_case(int32_t dw, int32_t sw, int clip, t_pair p)
{
	t_scene	dst;
	t_scene	src;
	t_blit	b;
	int		before;

	before = g_h.fail;
	scene_open(&dst, dw, dw, clip);
	scene_open(&src, sw, sw, 0);
	b = (t_blit){&dst.s, &src.s, p.dr, p.sr};
	if (p.mode)
	{
		gfx_blit_smooth(&b);
		scene_smooth(&dst, &src.s, p);
	}
	else
	{
		gfx_blit_scaled(&b);
		scene_scaled(&dst, &src.s, p);
	}
	scene_close(&dst, "blit mis a l'echelle contre le modele");
	scene_close(&src, "blit mis a l'echelle: source inchangee");
	if (g_h.fail != before)
		fprintf(stderr, "  dr(%d,%d,%d,%d) sr(%d,%d,%d,%d) mode %d\n", p.dr.x,
			p.dr.y, p.dr.w, p.dr.h, p.sr.x, p.sr.y, p.sr.w, p.sr.h, p.mode);
}

void	run_smooth(t_scene *dst, t_scene *src, t_rect dr, t_rect sr)
{
	t_blit	b;

	b = (t_blit){&dst->s, &src->s, dr, sr};
	gfx_blit_smooth(&b);
}
