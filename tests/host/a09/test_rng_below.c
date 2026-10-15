#include <stdio.h>
#include <stdlib.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "velum/random.h"

static double	chi_below(uint64_t bound, uint32_t draws)
{
	uint32_t	*count;
	double		expected;
	double		chi;
	double		gap;
	uint64_t	i;

	count = calloc(bound, sizeof(*count));
	if (!count)
		return (-1.0);
	i = 0;
	while (i < draws)
	{
		count[krandom_below(bound)]++;
		i++;
	}
	expected = (double)draws / (double)bound;
	chi = 0;
	i = 0;
	while (i < bound)
	{
		gap = (double)count[i++] - expected;
		chi += gap * gap / expected;
	}
	free(count);
	return (chi);
}

static void	uniformite_n3_n7_n1000(void)
{
	double	chi;

	fake_reset();
	chi = chi_below(3, 600000);
	printf("  chi-deux n=3 (2 ddl) : %.2f\n", chi);
	h_true(chi >= 0 && chi < 13.82, "n=3 hors du seuil p=0,001");
	chi = chi_below(7, 700000);
	printf("  chi-deux n=7 (6 ddl) : %.2f\n", chi);
	h_true(chi >= 0 && chi < 22.46, "n=7 hors du seuil p=0,001");
	chi = chi_below(1000, 1000000);
	printf("  chi-deux n=1000 (999 ddl) : %.2f\n", chi);
	h_true(chi > 850.0 && chi < 1143.0, "n=1000 hors de [850, 1143]");
}

static void	bornes_zero_un_deux_et_extremes(void)
{
	uint64_t	i;
	uint64_t	hits;

	fake_reset();
	h_eq_u64("n=0", krandom_below(0), 0);
	h_eq_u64("n=1", krandom_below(1), 0);
	hits = 0;
	i = 0;
	while (i < 200)
	{
		hits |= 1ull << krandom_below(2);
		h_true(krandom_below(0xffffffffffffffffull) != 0xffffffffffffffffull,
			"n=UINT64_MAX reste < n");
		h_true(krandom_below((1ull << 63) + 1) < (1ull << 63) + 1,
			"n=2^63+1 reste < n");
		i++;
	}
	h_eq_u64("n=2 donne 0 et 1", hits, 3);
}

static void	sans_biais_borne_trois_fois_deux_puissance_62(void)
{
	uint64_t	bound;
	uint64_t	low;
	uint64_t	i;

	fake_reset();
	bound = 3ull << 62;
	low = 0;
	i = 0;
	while (i < 200000)
	{
		low += krandom_below(bound) < (1ull << 62);
		i++;
	}
	printf("  part sous n/3 : %.4f (attendu 0.3333)\n", (double)low / 200000);
	h_true(low > 200000 * 0.325 && low < 200000 * 0.341,
		"part sous n/3 hors de [0,325 ; 0,341] : biais du modulo");
}

int	main(void)
{
	h_begin("a09/rng_below");
	h_run("below/uniformite-n3-n7-n1000", uniformite_n3_n7_n1000);
	h_run("below/bornes-0-1-2-max", bornes_zero_un_deux_et_extremes);
	h_run("below/sans-biais-3x2^62",
		sans_biais_borne_trois_fois_deux_puissance_62);
	return (h_end());
}
