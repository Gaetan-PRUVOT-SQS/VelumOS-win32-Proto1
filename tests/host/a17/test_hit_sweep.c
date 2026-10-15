#include "harness.h"
#include "lt_hit.h"

static void	sweep_style(t_rect outer, uint32_t style, int32_t step)
{
	t_lunawin	win;
	t_ltsweep	s;
	int			mx;

	mx = 0;
	while (mx < 2)
	{
		win = lt_win(outer, style, mx == 1);
		lt_sweep(&win, step, &s);
		lt_sweep_report(&win, &s);
		if (step == 1)
			lt_sweep_counts(&win, &s);
		mx++;
	}
}

static void	sweep_small(void)
{
	uint32_t	style;

	style = 0;
	while (style < 4096)
	{
		sweep_style(lp_rect(3, 7, 1, 1), style, 1);
		style++;
	}
	style = 0;
	while (style < 128)
	{
		sweep_style(lp_rect(3, 7, 100, 30), lt_rep_style(style), 1);
		style++;
	}
}

static void	sweep_large(void)
{
	static const uint32_t	keys[16] = {0x1f, 0x0f, 0x0b, 0x03, 0x01, 0x07,
		0x5f, 0x43, 0x41, 0x21f, 0x41f, 0x81f, 0x11, 0x00, 0x17, 0x1e};
	int						i;

	i = 0;
	while (i < 16)
	{
		sweep_style(lp_rect(-20, 15, 800, 600), keys[i], 9);
		i++;
	}
	sweep_style(lp_rect(0, 0, 800, 600), WS_DEFAULT, 1);
}

int	main(void)
{
	h_begin("a17/hit_sweep");
	h_run("1x1 (4096 styles) et 100x30 (classes d'equivalence)", sweep_small);
	h_run("800x600 : bords complets, interieur echantillonne", sweep_large);
	return (h_end());
}
