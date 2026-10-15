#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"
#include "velum/err.h"

static uint32_t	expected_mask(int sm, int rm, int now, int cyc)
{
	uint32_t	mask;

	mask = 0;
	if (sm == HW_OK)
		mask |= RNG_SRC_RDSEED;
	if (rm == HW_OK)
		mask |= RNG_SRC_RDRAND;
	if (now)
		mask |= RNG_SRC_TIME;
	if (cyc)
		mask |= RNG_SRC_JITTER;
	return (mask);
}

static uint32_t	gather_case(int idx, int *want_out)
{
	t_rng_env	env;
	uint8_t		seed[RNG_SEED_LEN];
	int			sm;
	int			rm;

	sm = idx % 3 + HW_ABSENT;
	rm = idx / 3 % 3 + HW_ABSENT;
	fake_reset();
	g_fake.rdseed.mode = sm;
	g_fake.rdrand.mode = rm;
	g_fake.now_ns = (uint64_t)(idx / 9 % 2) * 5;
	env = g_rng_env;
	if (idx / 18 == 0)
		env.cycles = NULL;
	*want_out = (int)expected_mask(sm, rm, idx / 9 % 2, idx / 18);
	return (rng_gather(&env, seed));
}

static void	table_de_decision_des_sources(void)
{
	int	idx;
	int	want;
	int	bad;

	bad = 0;
	idx = 0;
	while (idx < 36)
	{
		bad += (int)gather_case(idx, &want) != want;
		idx++;
	}
	h_eq_i64("combinaisons de sources en ecart sur 36", bad, 0);
}

int	main(void)
{
	h_begin("a09/rng_sources");
	h_run("gather/table-de-decision-rdseed-rdrand-horloge-gigue",
		table_de_decision_des_sources);
	return (h_end());
}
