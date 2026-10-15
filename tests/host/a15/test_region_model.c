#include <stdint.h>
#include <stdio.h>
#include "harness.h"
#include "ref.h"
#include "velum/gfx.h"

static void	model_exact_short(void)
{
	t_rgstat	st;

	st = (t_rgstat){0, 0, 0, 0};
	rg_sequences(20000, 3, &st);
	h_eq_i64("sequences courtes: aucun repli", st.fallback, 0);
	printf("  courtes: %lld pas, %lld exacts, n max %u\n", (long long)st.steps,
		(long long)st.exact, st.max_n);
}

static void	model_long(void)
{
	t_rgstat	st;

	st = (t_rgstat){0, 0, 0, 0};
	rg_sequences(100000, 10, &st);
	h_true(st.exact > 0, "des pas exacts");
	printf("  longues: %lld pas, %lld exacts, %lld replis, n max %u\n",
		(long long)st.steps, (long long)st.exact, (long long)st.fallback,
		st.max_n);
}

int	main(void)
{
	h_begin("a15/region_model");
	rng_seed(20261005);
	h_run("region: modele bitmap, sequences courtes exactes",
		model_exact_short);
	h_run("region: modele bitmap, 100000 sequences", model_long);
	return (h_end());
}
