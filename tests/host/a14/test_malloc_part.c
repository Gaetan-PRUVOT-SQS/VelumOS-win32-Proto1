#include <stdint.h>
#include "alloc_int.h"
#include "errno.h"
#include "fake_alloc.h"
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"
#include "velum/util.h"

#define SIZES_N 54

static const size_t	g_sizes[SIZES_N] = {0, 1, 15, 16, 17, 31, 32, 33, 47, 48,
	49, 63, 64, 65, 95, 96, 97, 127, 128, 129, 191, 192, 193, 255, 256, 257,
	383, 384, 385, 511, 512, 513, 767, 768, 769, 1023, 1024, 1025, 1535, 1536,
	1537, 2047, 2048, 2049, 4080, 4081, 5000, 65536, 100000, 1048576, 4095,
	4096, 4097, 8192};

static const size_t	g_class[ALLOC_NCLASS] = {16, 32, 48, 64, 96, 128, 192,
	256, 384, 512, 768, 1024, 1536, 2048};

static size_t	expected_cap(size_t n)
{
	size_t	i;

	if (!n)
		n = 1;
	i = 0;
	while (i < ALLOC_NCLASS && n > g_class[i])
		i++;
	if (i < ALLOC_NCLASS)
		return (g_class[i]);
	return (align_up(n + ALLOC_HDR, PAGE_SIZE) - ALLOC_HDR);
}

static void	part_boundaries(void)
{
	void	*p[SIZES_N];
	size_t	i;

	fake_reset();
	fake_kernel_on();
	i = 0;
	while (i < SIZES_N)
	{
		p[i] = malloc(g_sizes[i]);
		h_true(p[i] != NULL, "malloc non nul");
		h_true(((uintptr_t)p[i] & 15) == 0, "aligne sur 16");
		h_eq_u64("capacite = classe", alloc_capacity((t_ablock *)p[i] - 1),
			expected_cap(g_sizes[i]));
		fa_fill(p[i], g_sizes[i], (uint8_t)i);
		i++;
	}
	while (i--)
	{
		h_true(fa_intact(p[i], g_sizes[i], (uint8_t)i), "pas de recouvrement");
		free(p[i]);
	}
}

static void	part_zero_distinct(void)
{
	void	*a;
	void	*b;

	fake_reset();
	fake_kernel_on();
	a = malloc(0);
	b = malloc(0);
	h_true(a != NULL && b != NULL, "malloc(0) rend un bloc");
	h_true(a != b, "malloc(0) distincts");
	free(a);
	free(b);
}

static void	part_absurd(void)
{
	fake_reset();
	fake_kernel_on();
	errno = 0;
	h_true(malloc(ALLOC_MAX + 1) == NULL, "ALLOC_MAX + 1 refuse");
	h_eq_i64("errno ENOMEM", errno, ENOMEM);
	errno = 0;
	h_true(malloc(SIZE_MAX) == NULL, "SIZE_MAX refuse");
	h_eq_i64("errno ENOMEM (SIZE_MAX)", errno, ENOMEM);
	h_eq_u64("aucun appel noyau", g_fsys.calls, 0);
	g_fsys.valloc_budget = 0;
	errno = 0;
	h_true(malloc(ALLOC_MAX) == NULL, "ALLOC_MAX sans memoire");
	h_eq_i64("errno ENOMEM (noyau)", errno, ENOMEM);
}

int	main(void)
{
	h_begin("a14/malloc_part");
	h_run("malloc/limites : frontieres de classes", part_boundaries);
	h_run("malloc/partition : taille 0", part_zero_distinct);
	h_run("malloc/partition : tailles absurdes", part_absurd);
	return (h_end());
}
