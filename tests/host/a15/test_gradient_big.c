#include <stdint.h>
#include <string.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	big_one(t_rect r, bool horizontal, int clip)
{
	t_scene		sc;
	t_gradient	g;

	g.r = r;
	g.from = 0xff0058ee;
	g.to = 0xff3593ff;
	g.horizontal = horizontal;
	scene_open(&sc, 1024, 768, clip);
	gfx_gradient(&sc.s, &g);
	scene_gradient(&sc, &g);
	scene_close(&sc, "gradient 1024x768 contre la formule exacte");
}

static void	big_screen(void)
{
	big_one(rect_make(0, 0, 1024, 768), false, 0);
	big_one(rect_make(0, 0, 1024, 768), true, 0);
	big_one(rect_make(0, 0, 1024, 30), false, 0);
	big_one(rect_make(-40, -30, 2000, 1000), true, 2);
	big_one(rect_make(100, 200, 500, 300), false, 7);
}

static void	clip_invariance(void)
{
	t_scene		a;
	t_scene		b;
	t_gradient	g;
	int			k;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		g = grad_make(rect_make(-3, 2, 40, 14), 0xff204060, 0xffe0c080,
				(k & 1) != 0);
		scene_open(&a, 33, 20, k);
		scene_open(&b, 33, 20, 0);
		memcpy(b.g.px, a.g.px, 33 * 20 * sizeof(uint32_t));
		gfx_gradient(&a.s, &g);
		gfx_gradient(&b.s, &g);
		scene_copy_in(&a, &b);
		scene_close(&a, "gradient: le clip ne change pas les couleurs");
		scene_adopt(&b, rect_make(0, 0, 33, 20));
		scene_close(&b, "gradient: reference sans clip");
		k++;
	}
}

int	main(void)
{
	h_begin("a15/gradient_big");
	h_run("gradient: 1024x768 contre le modele", big_screen);
	h_run("gradient: invariance au clip", clip_invariance);
	return (h_end());
}
