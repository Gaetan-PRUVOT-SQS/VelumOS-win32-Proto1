#include <stdint.h>
#include <stdio.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define BIG_N 100000

static int	g_big[BIG_N];
static int	g_ref[2000];

static void	qr_insertion(int *v, size_t n)
{
	size_t	i;
	size_t	j;
	int		key;

	i = 1;
	while (i < n)
	{
		key = v[i];
		j = i;
		while (j > 0 && v[j - 1] > key)
		{
			v[j] = v[j - 1];
			j--;
		}
		v[j] = key;
		i++;
	}
}

static void	qr_big(void)
{
	uint64_t	before[2];
	uint64_t	after[2];
	uint64_t	bound;

	printf("graine qsort %llu\n", 0x5eed1234ull);
	f3_pattern(g_big, BIG_N, 6, 0x5eed1234ull);
	f3_hash(g_big, BIG_N, sizeof(int), before);
	f3_cmp_reset();
	qsort(g_big, BIG_N, sizeof(int), f3_cmp_counting);
	f3_hash(g_big, BIG_N, sizeof(int), after);
	bound = 40ull * BIG_N * 17;
	printf("comparaisons : %llu (borne %llu)\n",
		(unsigned long long)f3_cmp_calls(), (unsigned long long)bound);
	h_true(f3_is_sorted(g_big, BIG_N, sizeof(int), f3_cmp_int), "trie");
	h_true(before[0] == after[0] && before[1] == after[1], "permutation");
	h_true(f3_cmp_calls() < bound, "nombre de comparaisons raisonnable");
}

static void	qr_reference(void)
{
	static const size_t	sizes[8] = {2, 3, 5, 10, 50, 200, 1000, 2000};
	int					i;

	i = 0;
	while (i < 8)
	{
		f3_pattern(g_big, sizes[i], 6, 0x1000ull + (uint64_t)i);
		f3_pattern(g_ref, sizes[i], 6, 0x1000ull + (uint64_t)i);
		qr_insertion(g_ref, sizes[i]);
		qsort(g_big, sizes[i], sizeof(int), f3_cmp_int);
		h_true(f3_same_ints(g_big, g_ref, sizes[i]),
			"egal au tri par insertion");
		i++;
	}
}

static void	qr_metamorphic(void)
{
	static int	b[5000];
	size_t		i;

	f3_pattern(g_big, 5000, 6, 99);
	qsort(g_big, 5000, sizeof(int), f3_cmp_int);
	f3_pattern(b, 5000, 6, 99);
	i = 0;
	while (i < 2500)
	{
		b[i] ^= b[4999 - i];
		b[4999 - i] ^= b[i];
		b[i] ^= b[4999 - i];
		i++;
	}
	qsort(b, 5000, sizeof(int), f3_cmp_int);
	h_true(f3_same_ints(g_big, b, 5000), "entree inversee : meme resultat");
	qsort(b, 5000, sizeof(int), f3_cmp_int);
	h_true(f3_same_ints(g_big, b, 5000), "idempotent");
	qsort(b, 5000, sizeof(int), f3_cmp_int_desc);
	i = 0;
	while (i < 5000 && b[i] == g_big[4999 - i])
		i++;
	h_eq_u64("decroissant = croissant inverse", i, 5000);
}

int	main(void)
{
	h_begin("a14/qsort_random");
	h_run("qsort/aleatoire : 100 000 entiers", qr_big);
	h_run("qsort/metamorphique : egal au tri par insertion", qr_reference);
	h_run("qsort/metamorphique : inversion, idempotence, decroissant",
		qr_metamorphic);
	return (h_end());
}
