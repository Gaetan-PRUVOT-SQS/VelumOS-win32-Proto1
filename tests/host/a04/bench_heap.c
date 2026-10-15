#include <stdint.h>
#include <stdio.h>
#include <x86intrin.h>
#include "a04_fake.h"

#define BENCH_LOOPS 400000
#define BENCH_BATCH 600

static const uint32_t	g_sizes[] = {16, 32, 64, 128, 256, 512, 1024, 2048,
	4096, 65536};

static uint64_t	cycles_pair(size_t size)
{
	uint64_t	start;
	uint32_t	i;
	void		*p;

	i = 0;
	while (i < 1000)
	{
		kfree(kmalloc(size));
		i++;
	}
	start = __rdtsc();
	i = 0;
	while (i < BENCH_LOOPS)
	{
		p = kmalloc(size);
		kfree(p);
		i++;
	}
	return ((__rdtsc() - start) / BENCH_LOOPS);
}

static uint64_t	cycles_batch(size_t size)
{
	void		*p[BENCH_BATCH];
	uint64_t	start;
	uint32_t	i;
	uint32_t	r;

	start = __rdtsc();
	r = 0;
	while (r < 200)
	{
		i = 0;
		while (i < BENCH_BATCH)
			p[i++] = kmalloc(size);
		i = 0;
		while (i < BENCH_BATCH)
			kfree(p[i++]);
		r++;
	}
	return ((__rdtsc() - start) / (200ull * BENCH_BATCH));
}

static void	bench_cycles(void)
{
	uint32_t	i;

	i = 0;
	while (i < sizeof(g_sizes) / sizeof(g_sizes[0]))
	{
		a04_fresh();
		printf("a04 bench : %6u octets : %5llu cycles par paire "
			"kmalloc/kfree, %5llu cycles par operation en rafale\n", g_sizes[i],
			(unsigned long long)cycles_pair(g_sizes[i]),
			(unsigned long long)cycles_batch(g_sizes[i]) / 2);
		heap_trim();
		i++;
	}
}

static void	bench_fragmentation(void)
{
	uint64_t	size;
	uint64_t	asked;
	uint64_t	given;

	asked = 0;
	given = 0;
	size = 1;
	while (size <= 2048)
	{
		asked += size;
		given += a04_slot(size);
		size++;
	}
	printf("a04 bench : fragmentation interne moyenne 1..2048 octets : "
		"%.2f %% (%llu demandes, %llu reserves)\n", 100.0 * (given - asked)
		/ given, (unsigned long long)asked, (unsigned long long)given);
	printf("a04 bench : etat statique du tas : %zu octets\n", sizeof(g_heap));
}

int	main(void)
{
	bench_cycles();
	bench_fragmentation();
	return (0);
}
