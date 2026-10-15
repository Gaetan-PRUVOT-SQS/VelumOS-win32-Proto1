#include <stdint.h>
#include "harness.h"
#include "ppm.h"
#include "ref.h"
#include "velum/gfx.h"

static void	draw_region(t_surface *s, const t_region *rg, int32_t dx)
{
	static const t_color	pal[6] = {0xffe05050, 0xff50c050, 0xff5080e0,
		0xffe0c040, 0xffc060c0, 0xff40c0c0};
	uint32_t				i;
	t_rect					r;

	i = 0;
	while (i < rg->n)
	{
		r = rg->r[i];
		r.x += dx;
		gfx_fill(s, r, pal[i % 6]);
		gfx_frame(s, r, 0xff000000);
		i++;
	}
}

static void	visu_region(void)
{
	t_surface	s;
	t_region	rg;
	int32_t		i;

	visu_alloc(&s, 480, 100);
	gfx_fill(&s, rect_make(0, 0, 480, 100), 0xffffffff);
	region_clear(&rg);
	region_add(&rg, rect_make(10, 10, 50, 30));
	region_add(&rg, rect_make(40, 25, 50, 40));
	region_add(&rg, rect_make(90, 50, 40, 30));
	draw_region(&s, &rg, 0);
	region_subtract(&rg, rect_make(50, 20, 30, 50));
	draw_region(&s, &rg, 160);
	region_clear(&rg);
	i = 0;
	while (i < 40)
	{
		region_add(&rg, rect_make(4 * i, 4 * (i % 5) + 2, 3, 3));
		i++;
	}
	draw_region(&s, &rg, 320);
	h_eq_i64("region.png", visu_save("a15_regions", &s, 3), 0);
}

int	main(int argc, char **argv)
{
	h_begin("a15/visu_c");
	if (argc > 0)
		visu_init(argv[0]);
	h_run("images: regions", visu_region);
	return (h_end());
}
