#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bench.h"

uint64_t	bench_now(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ((uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec);
}

static int	by_value(const void *a, const void *b)
{
	uint64_t	x;
	uint64_t	y;

	x = *(const uint64_t *)a;
	y = *(const uint64_t *)b;
	if (x < y)
		return (-1);
	return (x > y);
}

void	bench_run(const t_bcase *c, t_bench *b)
{
	uint64_t	t[BENCH_REPS];
	uint64_t	t0;
	int			i;

	c->fn(b);
	i = 0;
	while (i < BENCH_REPS)
	{
		t0 = bench_now();
		c->fn(b);
		t[i] = bench_now() - t0;
		i++;
	}
	qsort(t, BENCH_REPS, sizeof(t[0]), by_value);
	printf("%-34s min %4llu.%03llu ms   mediane %4llu.%03llu ms\n", c->name,
		(unsigned long long)(t[0] / 1000000),
		(unsigned long long)(t[0] / 1000 % 1000),
		(unsigned long long)(t[BENCH_REPS / 2] / 1000000),
		(unsigned long long)(t[BENCH_REPS / 2] / 1000 % 1000));
}
