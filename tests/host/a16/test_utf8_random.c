#include <stdio.h>
#include <stdlib.h>
#include "a16_test.h"

#define CASES 10000

static const uint8_t	g_edge_bytes[] = {0x00, 0x41, 0x7F, 0x80, 0x8F, 0x90,
	0x9F, 0xA0, 0xBF, 0xC0, 0xC1, 0xC2, 0xDF, 0xE0, 0xE1, 0xEC, 0xED, 0xEE,
	0xEF, 0xF0, 0xF1, 0xF3, 0xF4, 0xF5, 0xFF};

static void	check_stream(const uint8_t *data, size_t len)
{
	uint8_t		*heap;
	const char	*cur;
	const char	*before;
	uint32_t	want;
	size_t		want_used;

	heap = exact_copy(data, len);
	cur = (const char *)heap;
	while (cur < (const char *)heap + len)
	{
		before = cur;
		want_used = ref_utf8_decode((const uint8_t *)before,
				(size_t)((const char *)heap + len - before), &want);
		h_eq_u64("ALEA-valeur egale au modele", font_utf8_next(&cur,
				(const char *)heap + len), want);
		h_true(cur > before, "ALEA-progression d'au moins un octet");
		h_true(cur <= (const char *)heap + len, "ALEA-jamais au-dela de end");
		h_eq_u64("ALEA-consomme egale au modele", (size_t)(cur - before),
			want_used);
	}
	free(heap);
}

static void	fuzz_pure_random(void)
{
	uint64_t	state;
	uint8_t		buf[16];
	size_t		len;
	size_t		i;
	int			n;

	state = rng_seed() ^ 0x1111;
	n = 0;
	while (n++ < CASES)
	{
		len = rng_next(&state) % 17;
		i = 0;
		while (i < len)
			buf[i++] = (uint8_t)(rng_next(&state) >> 24);
		check_stream(buf, len);
	}
}

static void	fuzz_boundary_biased(void)
{
	uint64_t	state;
	uint8_t		buf[16];
	size_t		len;
	size_t		i;
	int			n;

	state = rng_seed() ^ 0x2222;
	n = 0;
	while (n++ < CASES)
	{
		len = 1 + rng_next(&state) % 12;
		i = 0;
		while (i < len)
			buf[i++] = g_edge_bytes[(rng_next(&state) >> 20)
				% sizeof(g_edge_bytes)];
		check_stream(buf, len);
	}
}

int	main(void)
{
	printf("a16/utf8_random : graine 0x%llx (A16_SEED pour la changer)\n",
		(unsigned long long)rng_seed());
	h_begin("a16/utf8_random");
	h_run("10000 suites d'octets aleatoires", fuzz_pure_random);
	h_run("10000 suites d'octets de bordure", fuzz_boundary_biased);
	return (h_end());
}
