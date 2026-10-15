#include <stdint.h>
#include "fake_f3.h"
#include "harness.h"
#include "stdlib.h"

#define BS_N 100000

static int	g_big[BS_N];

static void	bs_argument_order(void)
{
	int	key;
	int	*p;

	f3_pattern(g_big, 1000, 0, 0);
	key = 500;
	f3_key_expect(&key);
	p = bsearch(&key, g_big, 1000, sizeof(int), f3_cmp_keyed);
	h_true(p == &g_big[500], "element trouve");
	h_eq_u64("la cle est toujours le premier argument", f3_key_violations(), 0);
	key = 5000;
	f3_key_expect(&key);
	h_true(bsearch(&key, g_big, 1000, sizeof(int), f3_cmp_keyed) == NULL,
		"absent");
	h_eq_u64("ordre des arguments, cas absent", f3_key_violations(), 0);
}

static int	bs_wrong(int key, uint64_t *worst)
{
	uint64_t	before;
	int			*p;
	int			expect;

	before = f3_cmp_calls();
	p = bsearch(&key, g_big, BS_N, sizeof(int), f3_cmp_counting);
	if (f3_cmp_calls() - before > *worst)
		*worst = f3_cmp_calls() - before;
	expect = key % 2 == 0 && key < 2 * BS_N;
	if (expect && p != &g_big[key / 2])
		return (1);
	return ((p != NULL) != expect);
}

static void	bs_large(void)
{
	int			i;
	int			bad;
	uint64_t	worst;

	i = 0;
	while (i < BS_N)
	{
		g_big[i] = 2 * i;
		i++;
	}
	bad = 0;
	worst = 0;
	i = 0;
	while (i < 2 * BS_N + 2)
		bad += bs_wrong(i++, &worst);
	h_eq_i64("pairs trouves, impairs et hors bornes absents", bad, 0);
	h_true(worst <= 17, "au plus 17 comparaisons par recherche");
}

int	main(void)
{
	h_begin("a14/bsearch_large");
	h_run("bsearch/exigence : ordre des arguments du comparateur",
		bs_argument_order);
	h_run("bsearch/limite : 100 000 elements, toutes les cles", bs_large);
	return (h_end());
}
