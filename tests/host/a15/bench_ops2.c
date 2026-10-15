#include "bench.h"

void	op_blit_alpha(t_bench *b)
{
	t_blit	bl;

	bl = (t_blit){&b->dst, &b->src, rect_make(0, 0, BENCH_W, BENCH_H_PX),
		rect_make(0, 0, BENCH_W, BENCH_H_PX)};
	gfx_blit_alpha(&bl);
}

void	op_scaled(t_bench *b)
{
	t_blit	bl;

	bl = (t_blit){&b->dst, &b->half, rect_make(0, 0, BENCH_W, BENCH_H_PX),
		rect_make(0, 0, BENCH_W / 2, BENCH_H_PX / 2)};
	gfx_blit_scaled(&bl);
}

void	op_smooth(t_bench *b)
{
	t_blit	bl;

	bl = (t_blit){&b->dst, &b->half, rect_make(0, 0, BENCH_W, BENCH_H_PX),
		rect_make(0, 0, BENCH_W / 2, BENCH_H_PX / 2)};
	gfx_blit_smooth(&bl);
}

void	op_gradient_v(t_bench *b)
{
	t_gradient	g;

	g = (t_gradient){rect_make(0, 0, BENCH_W, BENCH_H_PX), 0xff0058ee,
		0xff3593ff, false};
	gfx_gradient(&b->dst, &g);
}

void	op_gradient_h(t_bench *b)
{
	t_gradient	g;

	g = (t_gradient){rect_make(0, 0, BENCH_W, BENCH_H_PX), 0xff0058ee,
		0xff3593ff, true};
	gfx_gradient(&b->dst, &g);
}
