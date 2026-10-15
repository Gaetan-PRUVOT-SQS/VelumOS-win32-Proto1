#include <stdint.h>
#include "harness.h"
#include "ppm.h"
#include "ref.h"
#include "velum/gfx.h"

static const int8_t	g_fan[24][2] = {{44, 0}, {43, 11}, {38, 22}, {31, 31},
{22, 38}, {11, 43}, {0, 44}, {-11, 43}, {-22, 38}, {-31, 31}, {-38, 22},
{-43, 11}, {-44, 0}, {-43, -11}, {-38, -22}, {-31, -31}, {-22, -38},
{-11, -43}, {0, -44}, {11, -43}, {22, -38}, {31, -31}, {38, -22}, {43, -11}};

static void	visu_lines(void)
{
	t_surface	s;
	int			i;

	visu_alloc(&s, 128, 96);
	gfx_fill(&s, rect_make(0, 0, 128, 96), 0xffffffff);
	i = 0;
	while (i < 24)
	{
		gfx_line(&s, pt(64, 48), pt(64 + g_fan[i][0], 48 + g_fan[i][1]),
			gfx_argb(255, (uint8_t)(i * 10), (uint8_t)(255 - i * 10), 160));
		i++;
	}
	gfx_frame(&s, rect_make(1, 1, 126, 94), 0xff000000);
	gfx_hline(&s, pt(5, 5), 30, 0xff0000ff);
	gfx_vline(&s, pt(5, 5), 30, 0xffff0000);
	gfx_line(&s, pt(-20, -20), pt(160, 120), 0x80000000);
	gfx_line(&s, pt(0, 95), pt(127, 0), 0x80ff00ff);
	h_eq_i64("lignes.png", visu_save("a15_lignes", &s, 5), 0);
}

static void	visu_rrect(void)
{
	static const int32_t	radii[8] = {0, 1, 2, 3, 4, 5, 6, 8};
	t_surface				s;
	int						i;
	t_rrect					rr;

	visu_alloc(&s, 320, 100);
	gfx_fill(&s, rect_make(0, 0, 320, 100), 0xffece9d8);
	i = 0;
	while (i < 8)
	{
		rr = (t_rrect){rect_make(2 + i * 40, 2, 36, 36), 0xff245edc, radii[i],
			radii[i], radii[i], radii[i]};
		gfx_rrect_fill(&s, &rr);
		i++;
	}
	rr = (t_rrect){rect_make(4, 46, 120, 26), 0xff0058ee, 8, 8, 0, 0};
	gfx_rrect_fill(&s, &rr);
	rr = (t_rrect){rect_make(140, 46, 90, 26), 0xff3c8a2e, 13, 13, 13, 13};
	gfx_rrect_fill(&s, &rr);
	rr = (t_rrect){rect_make(246, 46, 70, 26), 0x80c00000, 12, 3, 12, 3};
	gfx_rrect_fill(&s, &rr);
	h_eq_i64("rrect.png", visu_save("a15_rrect", &s, 4), 0);
}

static void	visu_gradient(void)
{
	t_surface	s;
	t_gradient	g;

	visu_alloc(&s, 256, 120);
	gfx_fill(&s, rect_make(0, 0, 256, 120), 0xffece9d8);
	g = grad_make(rect_make(4, 4, 200, 28), 0xff0058ee, 0xff3593ff, false);
	gfx_gradient(&s, &g);
	g = grad_make(rect_make(4, 40, 100, 26), 0xff3c8a2e, 0xff5bba47, true);
	gfx_gradient(&s, &g);
	g = grad_make(rect_make(0, 74, 256, 12), 0xff000000, 0xffffffff, true);
	gfx_gradient(&s, &g);
	gfx_fill(&s, rect_make(120, 40, 130, 26), 0xff808080);
	g = grad_make(rect_make(120, 40, 130, 26), 0xffff0000, 0x000000ff, true);
	gfx_gradient(&s, &g);
	g = grad_make(rect_make(0, 92, 256, 28), 0xffffd000, 0xff2040ff, false);
	gfx_gradient(&s, &g);
	h_eq_i64("gradient.png", visu_save("a15_gradient", &s, 4), 0);
}

int	main(int argc, char **argv)
{
	h_begin("a15/visu_a");
	if (argc > 0)
		visu_init(argv[0]);
	h_run("images: lignes", visu_lines);
	h_run("images: rectangles arrondis", visu_rrect);
	h_run("images: degrades", visu_gradient);
	return (h_end());
}
