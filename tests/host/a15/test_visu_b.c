#include <stdint.h>
#include <stdlib.h>
#include "harness.h"
#include "ppm.h"
#include "ref.h"
#include "velum/gfx.h"

static void	checker(t_surface *s, t_rect r)
{
	int32_t	x;
	int32_t	y;
	t_color	c;

	y = r.y;
	while (y < r.y + r.h)
	{
		x = r.x;
		while (x < r.x + r.w)
		{
			c = 0xff808080;
			if ((((x - r.x) / 8 + (y - r.y) / 8) & 1) != 0)
				c = 0xffc0c0c0;
			gfx_fill(s, rect_make(x, y, 8, 8), c);
			x += 8;
		}
		y += 8;
	}
}

static void	visu_blend(void)
{
	t_surface	s;
	int			i;
	t_rrect		rr;

	visu_alloc(&s, 224, 80);
	checker(&s, rect_make(0, 0, 224, 80));
	i = 0;
	while (i < 8)
	{
		gfx_fill(&s, rect_make(4 + i * 27, 4, 24, 24),
			gfx_argb((uint8_t)(i * 36 + (i == 7) * 3), 220, 20, 20));
		i++;
	}
	gfx_line(&s, pt(0, 40), pt(223, 79), 0x800000ff);
	gfx_line(&s, pt(0, 79), pt(223, 40), 0x8000a000);
	rr = (t_rrect){rect_make(10, 36, 90, 36), 0xa0ffffff, 14, 14, 14, 14};
	gfx_rrect_fill(&s, &rr);
	h_eq_i64("blend.png", visu_save("a15_alpha", &s, 4), 0);
}

static void	sprite(t_surface *s)
{
	t_gradient	g;
	int32_t		i;

	g = grad_make(rect_make(0, 0, 40, 40), 0xff0058ee, 0xffffd000, false);
	gfx_gradient(s, &g);
	i = 0;
	while (i < 40)
	{
		gfx_fill(s, rect_make(i, i, 4, 4), 0xffffffff);
		gfx_put(s, pt(i, 39 - i), 0xff000000);
		i += 4;
	}
	gfx_frame(s, rect_make(0, 0, 40, 40), 0xff000000);
	gfx_frame(s, rect_make(10, 10, 20, 20), 0xffffffff);
}

static void	visu_blit(void)
{
	t_surface	s;
	t_surface	spr;
	t_blit		b;

	visu_alloc(&s, 340, 130);
	visu_alloc(&spr, 40, 40);
	gfx_fill(&s, rect_make(0, 0, 340, 130), 0xffece9d8);
	sprite(&spr);
	b = (t_blit){&s, &spr, rect_make(4, 4, 40, 40), rect_make(0, 0, 40, 40)};
	gfx_blit(&b);
	b.dr = rect_make(50, 4, 80, 80);
	gfx_blit_scaled(&b);
	b.dr = rect_make(136, 4, 80, 80);
	gfx_blit_smooth(&b);
	b.dr = rect_make(222, 4, 20, 20);
	gfx_blit_scaled(&b);
	b.dr = rect_make(246, 4, 20, 20);
	gfx_blit_smooth(&b);
	b.dr = rect_make(222, 30, 31, 31);
	gfx_blit_smooth(&b);
	h_eq_i64("blit.png", visu_save("a15_blit", &s, 3), 0);
	free(spr.px);
}

int	main(int argc, char **argv)
{
	h_begin("a15/visu_b");
	if (argc > 0)
		visu_init(argv[0]);
	h_run("images: alpha", visu_blend);
	h_run("images: blit, echelle, lissage", visu_blit);
	return (h_end());
}
