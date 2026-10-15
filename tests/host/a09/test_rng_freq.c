#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "velum/random.h"

#define SAMPLE 1048576

static double	chi_square_bytes(const uint8_t *buf, size_t n)
{
	uint32_t	count[256];
	double		expected;
	double		chi;
	double		gap;
	size_t		i;

	memset(count, 0, sizeof(count));
	i = 0;
	while (i < n)
	{
		count[buf[i]]++;
		i++;
	}
	expected = (double)n / 256.0;
	chi = 0;
	i = 0;
	while (i < 256)
	{
		gap = (double)count[i] - expected;
		chi += gap * gap / expected;
		i++;
	}
	return (chi);
}

static void	frequence_des_octets_chi_deux(void)
{
	uint8_t	*buf;
	double	chi;

	fake_reset();
	buf = malloc(SAMPLE);
	h_true(buf != NULL, "allocation de l'echantillon");
	if (!buf)
		return ;
	fill_random(buf, SAMPLE);
	chi = chi_square_bytes(buf, SAMPLE);
	printf("  chi-deux octets (255 ddl, 1 Mio) : %.1f\n", chi);
	h_true(chi > 190.0 && chi < 335.0, "chi-deux hors de [190, 335]");
	free(buf);
}

static void	bits_equilibres(void)
{
	uint8_t		*buf;
	uint64_t	ones;
	size_t		i;

	fake_reset();
	buf = malloc(SAMPLE);
	h_true(buf != NULL, "allocation de l'echantillon");
	if (!buf)
		return ;
	fill_random(buf, SAMPLE);
	ones = 0;
	i = 0;
	while (i < SAMPLE)
	{
		ones += (uint64_t)__builtin_popcount(buf[i]);
		i++;
	}
	printf("  bits a 1 sur %d : %llu\n", SAMPLE * 8, (unsigned long long)ones);
	h_true(ones > 4194304 - 7500 && ones < 4194304 + 7500,
		"bits a 1 hors de 4194304 +/- 7500");
	free(buf);
}

static void	deux_flux_de_graines_differentes_divergent(void)
{
	uint8_t	a[64];
	uint8_t	b[64];

	fake_reset();
	krandom(a, sizeof(a));
	fake_reset();
	g_fake.rng_state = 42;
	g_fake.tsc = 77;
	krandom(b, sizeof(b));
	h_true(memcmp(a, b, sizeof(a)) != 0, "graine differente, flux different");
}

int	main(void)
{
	h_begin("a09/rng_freq");
	h_run("rng/frequence-octets-chi2-1-mio", frequence_des_octets_chi_deux);
	h_run("rng/equilibre-des-bits-1-mio", bits_equilibres);
	h_run("rng/graines-differentes-flux-differents",
		deux_flux_de_graines_differentes_divergent);
	return (h_end());
}
