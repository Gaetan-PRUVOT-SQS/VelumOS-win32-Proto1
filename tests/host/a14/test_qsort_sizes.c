#include <stdint.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define SZ_N 300

static uint8_t	g_buf[SZ_N * 24];

static void	qs_sized(size_t size, int (*cmp)(const void *, const void *))
{
	uint64_t	rng;
	uint64_t	before[2];
	uint64_t	after[2];
	size_t		n;

	n = SZ_N;
	rng = 0xabcdef12345ull + size;
	f3_fill(g_buf, n * size, &rng);
	f3_hash(g_buf, n, size, before);
	qsort(g_buf, n, size, cmp);
	f3_hash(g_buf, n, size, after);
	h_true(f3_is_sorted(g_buf, n, size, cmp), "ordre selon memcmp");
	h_true(before[0] == after[0] && before[1] == after[1], "permutation");
}

static void	qs_element_sizes(void)
{
	qs_sized(1, f3_cmp_b1);
	qs_sized(3, f3_cmp_b3);
	qs_sized(8, f3_cmp_b8);
	qs_sized(24, f3_cmp_b24);
}

static void	qs_invalid(void)
{
	int	a[4];

	a[0] = 4;
	a[1] = 3;
	a[2] = 2;
	a[3] = 1;
	f3_cmp_reset();
	qsort(NULL, 4, sizeof(int), f3_cmp_counting);
	qsort(a, 4, sizeof(int), NULL);
	qsort(a, 4, 0, f3_cmp_counting);
	qsort(a, SIZE_MAX / 2 + 1, 1, f3_cmp_counting);
	qsort(a, SIZE_MAX / 2, 4, f3_cmp_counting);
	qsort(a, SIZE_MAX / 3, 8, f3_cmp_counting);
	h_eq_u64("aucune comparaison", f3_cmp_calls(), 0);
	h_true(a[0] == 4 && a[1] == 3 && a[2] == 2 && a[3] == 1, "tableau intact");
}

int	main(void)
{
	h_begin("a14/qsort_sizes");
	h_run("qsort/partition : elements de 1, 3, 8 et 24 octets",
		qs_element_sizes);
	h_run("qsort/supposition d'erreur : base, cmp, size nuls, produit",
		qs_invalid);
	return (h_end());
}
