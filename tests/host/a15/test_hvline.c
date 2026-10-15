#include <stdint.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	one_line(t_scene *sc, int vertical, t_point p, int32_t len)
{
	t_rect	model;

	model = rect_make(p.x, p.y, len, 1);
	if (vertical)
		model = rect_make(p.x, p.y, 1, len);
	if (vertical)
		gfx_vline(&sc->s, p, len, 0x90ff2020);
	else
		gfx_hline(&sc->s, p, len, 0x90ff2020);
	scene_rect(sc, model, 0x90ff2020);
}

static void	line_cases(int vertical)
{
	t_scene	sc;
	t_point	p;
	int32_t	len;
	int		k;

	k = 0;
	while (k < CLIP_VARIANTS)
	{
		len = -2;
		while (len < 10)
		{
			p.x = (k * 3) % 5 - 2;
			p.y = k % 6 - 1;
			scene_open(&sc, 7, 5, k);
			one_line(&sc, vertical, p, len);
			scene_close(&sc, "hline/vline equivalent a un fill 1 pixel");
			len++;
		}
		k++;
	}
}

static void	hline_cases(void)
{
	line_cases(0);
}

static void	vline_cases(void)
{
	line_cases(1);
}

int	main(void)
{
	h_begin("a15/hvline");
	h_run("hline: position x longueur x clip", hline_cases);
	h_run("vline: position x longueur x clip", vline_cases);
	return (h_end());
}
