#include "pmm_int.h"

static uint64_t	word_mask(uint64_t from, uint64_t to)
{
	if (to - from >= 64)
		return (~0ull);
	return (((1ull << (to - from)) - 1) << from);
}

static uint64_t	popcount(uint64_t x)
{
	x = x - ((x >> 1) & 0x5555555555555555ull);
	x = (x & 0x3333333333333333ull) + ((x >> 2) & 0x3333333333333333ull);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0full;
	return ((x * 0x0101010101010101ull) >> 56);
}

void	pmm_bits_fill(uint64_t from, uint64_t to, int one)
{
	uint64_t	w;
	uint64_t	last;
	uint64_t	mask;

	while (from < to)
	{
		w = from / 64;
		last = min_u64(to, (w + 1) * 64);
		mask = word_mask(from % 64, last - w * 64);
		if (one)
			g_pmm.bits[w] |= mask;
		else
			g_pmm.bits[w] &= ~mask;
		from = last;
	}
}

uint64_t	pmm_bits_next(uint64_t from, uint64_t to, int one)
{
	uint64_t	w;
	uint64_t	word;

	while (from < to)
	{
		w = from / 64;
		word = g_pmm.bits[w];
		if (!one)
			word = ~word;
		word &= ~0ull << (from % 64);
		g_pmm.scanned++;
		if (word)
			return (min_u64(w * 64 + __builtin_ctzll(word), to));
		from = (w + 1) * 64;
	}
	return (to);
}

uint64_t	pmm_bits_count_free(void)
{
	uint64_t	w;
	uint64_t	n;

	w = 0;
	n = 0;
	while (w < g_pmm.words)
	{
		n += 64 - popcount(g_pmm.bits[w]);
		w++;
	}
	return (n);
}
