#include <string.h>
#include "bench.h"
#include "gfx_int.h"

void	op_fill_loop(t_bench *b)
{
	int32_t	y;

	y = 0;
	while (y < BENCH_H_PX)
	{
		gfx_span_fill(gfx_px_at(&b->dst, 0, y), BENCH_W, 0xff3c8a2e);
		y++;
	}
}

void	op_fill_rep(t_bench *b)
{
	int32_t		y;
	uint32_t	*d;
	size_t		n;

	y = 0;
	while (y < BENCH_H_PX)
	{
		d = gfx_px_at(&b->dst, 0, y);
		n = BENCH_W;
		__asm__ volatile ("rep stosl" : "+D" (d), "+c" (n) : "a" (0xff3c8a2eu)
			: "memory");
		y++;
	}
}

void	op_copy_rep(t_bench *b)
{
	int32_t			y;
	uint32_t		*d;
	const uint32_t	*s;
	size_t			n;

	y = 0;
	while (y < BENCH_H_PX)
	{
		d = gfx_px_at(&b->dst, 0, y);
		s = gfx_px_at(&b->src, 0, y);
		n = BENCH_W;
		__asm__ volatile ("rep movsl" : "+D" (d), "+S" (s), "+c" (n) :
			: "memory");
		y++;
	}
}

void	op_copy_memcpy(t_bench *b)
{
	int32_t	y;

	y = 0;
	while (y < BENCH_H_PX)
	{
		memcpy(gfx_px_at(&b->dst, 0, y), gfx_px_at(&b->src, 0, y),
			BENCH_W * sizeof(uint32_t));
		y++;
	}
}

void	op_copy_loop(t_bench *b)
{
	int32_t			y;
	int32_t			x;
	uint32_t		*d;
	const uint32_t	*s;

	y = 0;
	while (y < BENCH_H_PX)
	{
		d = gfx_px_at(&b->dst, 0, y);
		s = gfx_px_at(&b->src, 0, y);
		x = 0;
		while (x < BENCH_W)
		{
			d[x] = s[x];
			x++;
		}
		y++;
	}
}
