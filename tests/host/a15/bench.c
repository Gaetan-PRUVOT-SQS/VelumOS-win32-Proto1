#include <stdio.h>
#include "bench.h"

static const t_bcase	g_cases[] = {
{"gfx_fill opaque 1024x768", op_fill},
{"gfx_fill alpha 128", op_fill_alpha},
{"gfx_blit copie", op_blit},
{"gfx_blit_alpha (alpha 128)", op_blit_alpha},
{"gfx_blit_scaled 512x384 -> 1024x768", op_scaled},
{"gfx_blit_smooth 512x384 -> 1024x768", op_smooth},
{"gfx_gradient vertical", op_gradient_v},
{"gfx_gradient horizontal", op_gradient_h},
{"gfx_rrect_fill plein ecran r=8", op_rrect},
{"gfx_scroll 16 lignes", op_scroll},
{"gfx_line x1000", op_lines},
{"variante: remplissage boucle C", op_fill_loop},
{"variante: remplissage rep stosl", op_fill_rep},
{"variante: copie memcpy", op_copy_memcpy},
{"variante: copie rep movsl", op_copy_rep},
{"variante: copie boucle C", op_copy_loop}};

int	main(void)
{
	t_bench	b;
	size_t	i;

	bench_open(&b);
	printf("a15/bench : %d repetitions par cas, ecran %dx%d\n", BENCH_REPS,
		BENCH_W, BENCH_H_PX);
	i = 0;
	while (i < sizeof(g_cases) / sizeof(g_cases[0]))
	{
		bench_run(&g_cases[i], &b);
		i++;
	}
	bench_close(&b);
	return (0);
}
