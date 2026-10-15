#include <stdio.h>
#include "harness.h"
#include "time_int.h"
#include "velum/err.h"

#define POINTS 1000000000ull
#define NFREQ 8
#define SEED 0x5eed0a05c0ffee11ull
#define TEN_YEARS_S 315360000ull

static const uint64_t	g_freq[NFREQ] = {1000000ull, 1193182ull, 14318180ull,
	24000000ull, 999999999ull, 2400000000ull, 3600000000ull, 10000000000ull};

static uint64_t	rng(uint64_t *s)
{
	*s ^= *s >> 12;
	*s ^= *s << 25;
	*s ^= *s >> 27;
	return (*s * 0x2545f4914f6cdd1dull);
}

static uint64_t	conv_one_freq(uint64_t f, uint32_t shift, uint64_t *s)
{
	t_clockconv	c;
	uint64_t	bad;
	uint64_t	cyc;
	uint64_t	n;
	t_u128		diff;

	bad = 0;
	n = POINTS / NFREQ;
	clockconv_init(&c, NS_PER_S, f);
	while (n > 0)
	{
		cyc = rng(s) >> shift;
		diff = (t_u128)clockconv_apply(&c, cyc) * f;
		if (diff > (t_u128)cyc * NS_PER_S)
			diff -= (t_u128)cyc * NS_PER_S;
		else
			diff = (t_u128)cyc * NS_PER_S - diff;
		bad += (diff > (t_u128)cyc * 1000 + f);
		n--;
	}
	return (bad);
}

static void	conv_random(void)
{
	uint64_t	seed;
	uint64_t	bad;
	uint64_t	span;
	uint32_t	shift;
	uint32_t	k;

	seed = SEED;
	printf("graine %#llx, %llu points\n", (unsigned long long)seed,
		(unsigned long long)POINTS);
	bad = 0;
	k = 0;
	while (k < NFREQ)
	{
		span = g_freq[k] * TEN_YEARS_S;
		shift = 64;
		while (span > 1)
		{
			span >>= 1;
			shift--;
		}
		bad += conv_one_freq(g_freq[k], shift, &seed);
		k++;
	}
	h_eq_u64("points hors 1 ppm + 1 ns", bad, 0);
}

static void	conv_errors(void)
{
	t_clockconv	c;

	h_eq_i64("diviseur nul", clockconv_init(&c, NS_PER_S, 0), E_INVAL);
	h_eq_i64("numerateur nul", clockconv_init(&c, 0, 1), E_INVAL);
	h_eq_i64("multiplicateur nul", clockconv_init(&c, 1, 1ull << 40),
		E_RANGE);
	h_eq_i64("multiplicateur trop grand", clockconv_init(&c, 1ull << 40, 1),
		E_RANGE);
	h_eq_i64("1 ghz exact", clockconv_init(&c, NS_PER_S, NS_PER_S), E_OK);
	h_eq_u64("1 ghz identite", clockconv_apply(&c, 123456789), 123456789);
	clockconv_init(&c, NS_PER_S, 1);
	h_eq_u64("saturation", clockconv_apply(&c, UINT64_MAX), UINT64_MAX);
	h_eq_u64("ref nulle", hz_from_ref(5, 0, PIT_HZ), 0);
	h_eq_u64("ref pit", hz_from_ref(3000000000ull, PIT_HZ, PIT_HZ),
		3000000000ull);
	h_eq_u64("ref sature", hz_from_ref(UINT64_MAX, 1, UINT64_MAX),
		UINT64_MAX);
	h_true(hz_coherent(1010000000, 1000000000, 10000), "ecart +1 % accepte");
	h_true(!hz_coherent(1010000001, 1000000000, 10000), "ecart +1 % + 1");
	h_true(hz_coherent(990000000, 1000000000, 10000), "ecart -1 % accepte");
	h_true(!hz_coherent(989999999, 1000000000, 10000), "ecart -1 % - 1");
	h_true(!hz_coherent(0, 1000000000, 10000), "frequence nulle");
	h_eq_u64("cpuid 15 cristal", hz_from_cpuid15(2, 176, 24000000),
		2112000000ull);
	h_eq_u64("cpuid 15 vide", hz_from_cpuid15(2, 176, 0), 0);
}

int	main(void)
{
	h_begin("a05/tsc_conv");
	h_run("conversions et erreurs", conv_errors);
	h_run("1e9 points aleatoires", conv_random);
	return (h_end());
}
