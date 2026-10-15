#include <stdint.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

static int	g_ten[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

static void	bs_trivial(void)
{
	int	one;
	int	key;

	one = 5;
	key = 5;
	h_true(bsearch(&key, &one, 0, sizeof(int), f3_cmp_int) == NULL, "vide");
	h_true(bsearch(&key, &one, 1, sizeof(int), f3_cmp_int) == &one,
		"un, present");
	key = 6;
	h_true(bsearch(&key, &one, 1, sizeof(int), f3_cmp_int) == NULL,
		"un, absent");
	key = 5;
	h_true(bsearch(&key, &one, 1, 0, f3_cmp_int) == NULL, "size 0");
	h_true(bsearch(&key, NULL, 1, sizeof(int), f3_cmp_int) == NULL,
		"base NULL");
	h_true(bsearch(&key, &one, 1, sizeof(int), NULL) == NULL, "cmp NULL");
}

static void	bs_positions(void)
{
	int	key;
	int	i;

	i = 0;
	while (i < 10)
	{
		key = g_ten[i];
		h_true(bsearch(&key, g_ten, 10, sizeof(int), f3_cmp_int) == &g_ten[i],
			"present a chaque position");
		i++;
	}
	key = 5;
	h_true(bsearch(&key, g_ten, 10, sizeof(int), f3_cmp_int) == NULL,
		"absent avant le premier");
	key = 55;
	h_true(bsearch(&key, g_ten, 10, sizeof(int), f3_cmp_int) == NULL,
		"absent entre deux");
	key = 101;
	h_true(bsearch(&key, g_ten, 10, sizeof(int), f3_cmp_int) == NULL,
		"absent apres le dernier");
}

static void	bs_duplicates(void)
{
	static int	d[7] = {1, 2, 2, 2, 2, 3, 3};
	int			key;
	int			*p;

	key = 2;
	p = bsearch(&key, d, 7, sizeof(int), f3_cmp_int);
	h_true(p != NULL && p >= &d[1] && p <= &d[4] && *p == 2,
		"un des egaux est rendu");
	key = 3;
	p = bsearch(&key, d, 7, sizeof(int), f3_cmp_int);
	h_true(p != NULL && p >= &d[5] && *p == 3, "doublons en fin");
	key = 1;
	p = bsearch(&key, d, 7, sizeof(int), f3_cmp_int);
	h_true(p == &d[0], "premier unique");
}

int	main(void)
{
	h_begin("a14/bsearch");
	h_run("bsearch/partition : vide, un element, parametres nuls", bs_trivial);
	h_run("bsearch/limite : positions, absents", bs_positions);
	h_run("bsearch/partition : doublons", bs_duplicates);
	return (h_end());
}
