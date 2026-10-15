#include "bench.h"

void	op_rrect(t_bench *b)
{
	t_rrect	rr;

	rr = (t_rrect){rect_make(0, 0, BENCH_W, BENCH_H_PX), 0xff245edc, 8, 8, 8,
		8};
	gfx_rrect_fill(&b->dst, &rr);
}

void	op_scroll(t_bench *b)
{
	gfx_scroll(&b->dst, rect_make(0, 0, BENCH_W, BENCH_H_PX),
		(t_point){0, -16});
}

void	op_lines(t_bench *b)
{
	int32_t	i;

	i = 0;
	while (i < 1000)
	{
		gfx_line(&b->dst, (t_point){(i * 37) % BENCH_W, (i * 91) % BENCH_H_PX},
			(t_point){(i * 53 + 500) % BENCH_W, (i * 29 + 300) % BENCH_H_PX},
			0xffffffff);
		i++;
	}
}
