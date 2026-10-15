#include <stdio.h>
#include <stdlib.h>
#include "bench.h"

void	bench_open(t_bench *b)
{
	size_t	n;
	size_t	i;

	n = (size_t)BENCH_W * BENCH_H_PX;
	b->mem = aligned_alloc(64, n * 3 * sizeof(uint32_t));
	if (b->mem == NULL)
		abort();
	i = 0;
	while (i < n * 3)
	{
		b->mem[i] = ((uint32_t)(i * 2654435761u) & 0x00ffffffu) | 0x80000000u;
		i++;
	}
	gfx_surface_init(&b->dst, b->mem, BENCH_W, BENCH_H_PX);
	gfx_surface_init(&b->src, b->mem + n, BENCH_W, BENCH_H_PX);
	gfx_surface_init(&b->half, b->mem + 2 * n, BENCH_W / 2, BENCH_H_PX / 2);
}

void	bench_close(t_bench *b)
{
	free(b->mem);
}

void	op_fill(t_bench *b)
{
	gfx_fill(&b->dst, rect_make(0, 0, BENCH_W, BENCH_H_PX), 0xff3c8a2e);
}

void	op_fill_alpha(t_bench *b)
{
	gfx_fill(&b->dst, rect_make(0, 0, BENCH_W, BENCH_H_PX), 0x803c8a2e);
}

void	op_blit(t_bench *b)
{
	t_blit	bl;

	bl = (t_blit){&b->dst, &b->src, rect_make(0, 0, BENCH_W, BENCH_H_PX),
		rect_make(0, 0, BENCH_W, BENCH_H_PX)};
	gfx_blit(&bl);
}
