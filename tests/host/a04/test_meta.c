#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "a04_fake.h"

static const uint64_t	g_seeds[A04_SEEDS] = {0x1badb002, 0x2545f4914f6cdd1d,
	0x9e3779b97f4a7c15, 7, 0xdeadbeefcafe, 0x5eed2026};

static uint64_t	first_seed(void)
{
	const char	*env;

	env = getenv("A04_SEED");
	if (env)
		return (strtoull(env, NULL, 0));
	return (g_seeds[0]);
}

static void	meta_random_sequences(void)
{
	uint32_t	i;
	uint64_t	seed;

	i = 0;
	while (i < A04_SEEDS)
	{
		seed = g_seeds[i];
		if (i == 0)
			seed = first_seed();
		printf("a04 meta : graine %#llx, 30000 operations\n",
			(unsigned long long)seed);
		a04_meta_run(seed, 30000);
		i++;
	}
}

static void	meta_same_seed_same_layout(void)
{
	uint64_t	a;
	uint64_t	b;

	a = a04_meta_run(0x1234abcd, 8000);
	b = a04_meta_run(0x1234abcd, 8000);
	h_eq_u64("E12 meme graine : meme disposition des adresses", a, b);
	b = a04_meta_run(0x1234abce, 8000);
	h_true(a != b, "E12 autre graine : autre disposition");
}

static void	meta_short_sequences_every_length(void)
{
	uint64_t	n;

	n = 0;
	while (n < 40)
	{
		a04_meta_run(0xc0ffee + n, n);
		n++;
	}
}

int	main(void)
{
	h_begin("a04/metamorphique");
	h_run("E12 sequences aleatoires a graine affichee", meta_random_sequences);
	h_run("E12 determinisme", meta_same_seed_same_layout);
	h_run("E12 sequences courtes", meta_short_sequences_every_length);
	return (h_end());
}
