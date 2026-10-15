#include <stdint.h>
#include <stdio.h>
#include "fake_f4.h"
#include "harness.h"

#define FUZZ_SEED 0x5a17c0deull
#define FUZZ_ROUNDS 100000

static void	fuzz_run(int mode, uint64_t seed)
{
	t_f4_stack	s;
	uint64_t	rng;
	int			bad;
	int			i;

	rng = seed;
	bad = 0;
	i = 0;
	while (i++ < FUZZ_ROUNDS)
	{
		if (mode == 2)
			f4_fill_mutated(&s, &rng);
		else
			f4_fill_random(&s, &rng, mode);
		bad += f4_fuzz_one(&s);
	}
	h_eq_i64("ecarts au modele ou lecture hors fenetre", bad, 0);
}

static void	fuzz_all(void)
{
	printf("graine fuzz start_parse %llu\n", FUZZ_SEED);
	fuzz_run(0, FUZZ_SEED);
	fuzz_run(1, FUZZ_SEED + 1);
	fuzz_run(2, FUZZ_SEED + 2);
	printf("codes : OK %d, TRONQUE %d, INVAL %d, RANGE %d, PROTO %d\n",
		f4_hist(0), f4_hist(1), f4_hist(2), f4_hist(3), f4_hist(4));
	h_true(f4_hist(0) > 0 && f4_hist(1) > 0, "codes 0 et 1 atteints");
	h_true(f4_hist(2) > 0 && f4_hist(3) > 0 && f4_hist(4) > 0,
		"codes d'erreur tous atteints");
}

int	main(void)
{
	h_begin("a14/start_parse_fuzz");
	h_run("start_parse/aleatoire : 3 modes x 100 000 piles", fuzz_all);
	return (h_end());
}
