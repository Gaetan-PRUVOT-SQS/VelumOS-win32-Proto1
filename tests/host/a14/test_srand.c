#include <stdint.h>
#include "harness.h"
#include "stdlib.h"

#define SR_N 100

static int	g_a[SR_N];
static int	g_b[SR_N];

static void	srand_fill(unsigned int seed, int *v, int n)
{
	int	i;

	srand(seed);
	i = 0;
	while (i < n)
	{
		v[i] = rand();
		i++;
	}
}

static int	srand_equal(const int *a, const int *b, int n)
{
	int	i;

	i = 0;
	while (i < n && a[i] == b[i])
		i++;
	return (i == n);
}

static void	srand_repro(void)
{
	srand_fill(42, g_a, SR_N);
	srand_fill(42, g_b, SR_N);
	h_true(srand_equal(g_a, g_b, SR_N), "meme graine, meme suite");
	srand_fill(0, g_a, SR_N);
	srand_fill(0, g_b, SR_N);
	h_true(srand_equal(g_a, g_b, SR_N), "graine 0 reproductible");
	srand_fill(4294967295u, g_a, SR_N);
	srand_fill(4294967295u, g_b, SR_N);
	h_true(srand_equal(g_a, g_b, SR_N), "graine UINT_MAX reproductible");
}

static void	srand_distinct(void)
{
	int	bad;
	int	i;

	srand_fill(1, g_a, SR_N);
	srand_fill(2, g_b, SR_N);
	h_true(!srand_equal(g_a, g_b, SR_N), "graines 1 et 2 : suites differentes");
	srand_fill(0, g_b, SR_N);
	h_true(!srand_equal(g_a, g_b, SR_N), "graines 0 et 1 : suites differentes");
	bad = 0;
	i = 0;
	while (i < SR_N)
	{
		bad += g_b[i] < 0 || g_a[i] < 0;
		i++;
	}
	h_eq_i64("valeurs dans [0, RAND_MAX]", bad, 0);
}

int	main(void)
{
	h_begin("a14/srand");
	h_run("srand/exigence : suite reproductible", srand_repro);
	h_run("srand/partition : graines distinctes", srand_distinct);
	return (h_end());
}
