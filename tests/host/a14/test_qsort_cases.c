#include <stdint.h>
#include <stdio.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

static int	g_v[1000];

static void	qs_run(size_t n, int mode, int desc, uint64_t seed)
{
	uint64_t	before[2];
	uint64_t	after[2];
	int			(*cmp)(const void *, const void *);
	int			ok;

	cmp = f3_cmp_int;
	if (desc)
		cmp = f3_cmp_int_desc;
	f3_pattern(g_v, n, mode, seed);
	f3_hash(g_v, n, sizeof(int), before);
	qsort(g_v, n, sizeof(int), cmp);
	f3_hash(g_v, n, sizeof(int), after);
	ok = f3_is_sorted(g_v, n, sizeof(int), cmp) && before[0] == after[0]
		&& before[1] == after[1];
	h_true(ok, "qsort : trie et permutation");
	if (!ok)
		fprintf(stderr, "  n %zu motif %d desc %d\n", n, mode, desc);
}

static void	qs_trivial(void)
{
	int	a[3];

	a[0] = 3;
	a[1] = 1;
	a[2] = 2;
	f3_cmp_reset();
	qsort(a, 0, sizeof(int), f3_cmp_counting);
	qsort(a, 1, sizeof(int), f3_cmp_counting);
	h_eq_u64("aucune comparaison pour 0 ou 1 element", f3_cmp_calls(), 0);
	h_true(a[0] == 3 && a[1] == 1 && a[2] == 2, "tableau intact");
	qsort(a, 2, sizeof(int), f3_cmp_counting);
	h_true(a[0] == 1 && a[1] == 3 && a[2] == 2, "deux elements echanges");
	h_eq_u64("une comparaison pour 2 elements", f3_cmp_calls(), 1);
}

static void	qs_small(void)
{
	int	n;
	int	mode;

	n = 2;
	while (n <= 9)
	{
		mode = 0;
		while (mode <= 6)
		{
			qs_run((size_t)n, mode, 0, (uint64_t)n * 77 + 1);
			qs_run((size_t)n, mode, 1, (uint64_t)n * 77 + 2);
			mode++;
		}
		n++;
	}
}

static void	qs_patterns(void)
{
	static const size_t	sizes[3] = {10, 100, 1000};
	int					s;
	int					mode;

	s = 0;
	while (s < 3)
	{
		mode = 0;
		while (mode <= 6)
		{
			qs_run(sizes[s], mode, 0, 0x5eedull + (uint64_t)mode);
			qs_run(sizes[s], mode, 1, 0x5eedull + (uint64_t)mode);
			mode++;
		}
		s++;
	}
}

int	main(void)
{
	h_begin("a14/qsort_cases");
	h_run("qsort/partition : 0, 1 et 2 elements", qs_trivial);
	h_run("qsort/limite : tailles 2 a 9, sept motifs", qs_small);
	h_run("qsort/partition : trie, inverse, egaux, doublons, aleatoire",
		qs_patterns);
	return (h_end());
}
