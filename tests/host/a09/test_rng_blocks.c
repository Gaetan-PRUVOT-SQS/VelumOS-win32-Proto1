#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "velum/random.h"

#define SAMPLE 1048576
#define BLOCK 64

static uint64_t	block_hash(const uint8_t *p)
{
	uint64_t	h;
	size_t		i;

	h = 0xcbf29ce484222325ull;
	i = 0;
	while (i < BLOCK)
	{
		h ^= p[i++];
		h *= 0x100000001b3ull;
	}
	return (h);
}

static int	compare_u64(const void *a, const void *b)
{
	uint64_t	x;
	uint64_t	y;

	x = *(const uint64_t *)a;
	y = *(const uint64_t *)b;
	return ((x > y) - (x < y));
}

static int	count_duplicates(uint64_t *hashes, size_t n)
{
	size_t	i;
	int		dup;

	qsort(hashes, n, sizeof(*hashes), compare_u64);
	dup = 0;
	i = 1;
	while (i < n)
	{
		dup += hashes[i] == hashes[i - 1];
		i++;
	}
	return (dup);
}

static void	aucun_bloc_de_64_octets_repete(void)
{
	uint8_t		*buf;
	uint64_t	*hashes;
	size_t		i;

	fake_reset();
	buf = malloc(SAMPLE);
	hashes = malloc(SAMPLE / BLOCK * sizeof(*hashes));
	h_true(buf && hashes, "allocation de l'echantillon");
	if (!buf || !hashes)
		return ;
	fill_random(buf, SAMPLE);
	i = 0;
	while (i < SAMPLE / BLOCK)
	{
		hashes[i] = block_hash(buf + i * BLOCK);
		i++;
	}
	h_eq_i64("blocs de 64 octets en double sur 16384",
		count_duplicates(hashes, SAMPLE / BLOCK), 0);
	free(buf);
	free(hashes);
}

int	main(void)
{
	h_begin("a09/rng_blocks");
	h_run("rng/blocs-de-64-octets-uniques-1-mio",
		aucun_bloc_de_64_octets_repete);
	return (h_end());
}
