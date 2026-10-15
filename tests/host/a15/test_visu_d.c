#include <stdint.h>
#include "harness.h"
#include "ppm.h"
#include "ref.h"
#include "velum/gfx.h"

static void	wallpaper(t_surface *s)
{
	t_gradient	g;
	t_rrect		hill;

	g = grad_make(rect_make(0, 0, 1024, 480), 0xff4a86d8, 0xffb4d4f4, false);
	gfx_gradient(s, &g);
	hill = (t_rrect){rect_make(-300, 400, 1100, 800), 0xff4c9a38, 400, 400, 0,
		0};
	gfx_rrect_fill(s, &hill);
	hill = (t_rrect){rect_make(300, 470, 1100, 800), 0xff3c8a2e, 400, 400, 0,
		0};
	gfx_rrect_fill(s, &hill);
	gfx_fill(s, rect_make(80, 70, 90, 22), 0xc0ffffff);
	gfx_fill(s, rect_make(100, 56, 50, 16), 0xc0ffffff);
}

static void	taskbar(t_surface *s)
{
	t_gradient	g;
	t_rrect		start;

	g = grad_make(rect_make(0, 738, 1024, 30), 0xff3f8cf3, 0xff245edc, false);
	gfx_gradient(s, &g);
	gfx_hline(s, pt(0, 738), 1024, 0xff6fa8ff);
	start = (t_rrect){rect_make(0, 738, 110, 30), 0xff3c8a2e, 0, 12, 12, 0};
	gfx_rrect_fill(s, &start);
	g = grad_make(rect_make(0, 740, 100, 24), 0x405bba47, 0x00000000, false);
	gfx_gradient(s, &g);
}

static void	window(t_surface *s)
{
	t_rrect		w;
	t_gradient	g;

	w = (t_rrect){rect_make(204, 124, 520, 360), 0x50000000, 10, 10, 4, 4};
	gfx_rrect_fill(s, &w);
	w = (t_rrect){rect_make(200, 120, 520, 360), 0xff0055ea, 8, 8, 0, 0};
	gfx_rrect_fill(s, &w);
	g = grad_make(rect_make(208, 120, 504, 30), 0xff0058ee, 0xff3593ff, false);
	gfx_gradient(s, &g);
	gfx_fill(s, rect_make(204, 150, 512, 326), 0xffece9d8);
	gfx_frame(s, rect_make(204, 150, 512, 326), 0xff808080);
	w = (t_rrect){rect_make(228, 186, 90, 26), 0xffece9d8, 3, 3, 3, 3};
	gfx_rrect_fill(s, &w);
	gfx_frame(s, rect_make(228, 230, 200, 14), 0xff808080);
	gfx_fill(s, rect_make(230, 232, 120, 10), 0xff3ea03a);
}

static void	visu_desktop(void)
{
	t_surface	s;
	t_surface	icon;
	t_blit		b;

	visu_alloc(&s, 1024, 768);
	wallpaper(&s);
	window(&s);
	taskbar(&s);
	icon = gfx_surface_sub(&s, rect_make(210, 156, 32, 32));
	b = (t_blit){&s, &icon, rect_make(600, 200, 96, 96), rect_make(0, 0, 32,
			32)};
	gfx_blit_smooth(&b);
	b.dr = rect_make(600, 320, 96, 96);
	gfx_blit_scaled(&b);
	h_eq_i64("bureau.png", visu_save("a15_bureau", &s, 1), 0);
}

int	main(int argc, char **argv)
{
	h_begin("a15/visu_d");
	if (argc > 0)
		visu_init(argv[0]);
	h_run("images: composition type bureau 1024x768", visu_desktop);
	return (h_end());
}
