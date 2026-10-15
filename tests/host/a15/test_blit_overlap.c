#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	self_blit(int clip, t_point d, int alpha)
{
	t_scene		sc;
	t_surface	snap;
	uint32_t	*copy;
	t_pair		p;
	t_blit		b;

	scene_open(&sc, 9, 7, clip);
	copy = scene_snapshot(&sc);
	gfx_surface_init(&snap, copy, 9, 7);
	p = (t_pair){rect_make(d.x, d.y, 6, 4), rect_make(0, 0, 6, 4), alpha};
	b = (t_blit){&sc.s, &sc.s, p.dr, p.sr};
	if (alpha)
		gfx_blit_alpha(&b);
	else
		gfx_blit(&b);
	scene_blit(&sc, &snap, p);
	scene_close(&sc, "blit: recouvrement de la meme surface");
	free(copy);
}

static void	same_surface(void)
{
	int32_t	dx;
	int32_t	dy;
	int		k;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		dy = -5;
		while (dy <= 5)
		{
			dx = -5;
			while (dx <= 5)
			{
				self_blit(k, pt(dx, dy), (dx + dy + k) & 1);
				dx++;
			}
			dy++;
		}
		k++;
	}
}

static void	sub_blit(t_point a, t_point b, int to_b)
{
	t_scene		sc;
	t_surface	snap;
	t_surface	sa;
	t_surface	sb;
	t_blit		bl;

	scene_open(&sc, 16, 10, 0);
	sa = gfx_surface_sub(&sc.s, rect_make(a.x, a.y, 10, 8));
	sb = gfx_surface_sub(&sc.s, rect_make(b.x, b.y, 10, 8));
	gfx_surface_init(&snap, scene_snapshot(&sc), 16, 10);
	bl = (t_blit){&sb, &sa, rect_make(0, 0, 10, 8), rect_make(0, 0, 10, 8)};
	if (!to_b)
		bl = (t_blit){&sa, &sb, bl.dr, bl.sr};
	gfx_blit(&bl);
	if (to_b)
		scene_blit(&sc, &snap, (t_pair){rect_make(b.x, b.y, 10, 8),
			rect_make(a.x, a.y, 10, 8), 0});
	else
		scene_blit(&sc, &snap, (t_pair){rect_make(a.x, a.y, 10, 8),
			rect_make(b.x, b.y, 10, 8), 0});
	scene_close(&sc, "blit: sous-surfaces qui se recouvrent");
	free(snap.px);
}

static void	sub_surfaces(void)
{
	sub_blit(pt(0, 0), pt(3, 2), 1);
	sub_blit(pt(0, 0), pt(3, 2), 0);
	sub_blit(pt(0, 0), pt(1, 0), 1);
	sub_blit(pt(0, 0), pt(1, 0), 0);
	sub_blit(pt(2, 1), pt(0, 0), 1);
	sub_blit(pt(0, 1), pt(0, 0), 1);
	sub_blit(pt(0, 0), pt(0, 2), 0);
	sub_blit(pt(6, 2), pt(0, 2), 1);
}

int	main(void)
{
	h_begin("a15/blit_overlap");
	rng_seed(20261005);
	h_run("blit: meme surface, 8 directions x clips", same_surface);
	h_run("blit: sous-surfaces partageant les pixels", sub_surfaces);
	return (h_end());
}
