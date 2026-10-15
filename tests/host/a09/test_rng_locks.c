#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "velum/random.h"

static void	verrous_equilibres_apres_usage_mixte(void)
{
	uint8_t	buf[100];
	int		i;

	fake_reset();
	i = 0;
	while (i < 100)
	{
		krandom(buf, (size_t)(i % 70));
		random_add_entropy(buf, (size_t)(i % 33));
		krandom_below((uint64_t)i + 1);
		krandom_u64();
		i++;
	}
	h_eq_i64("erreurs de verrou", g_fake.lock_errors, 0);
	h_eq_i64("profondeur de verrou residuelle", g_fake.lock_depth, 0);
	h_eq_i64("interruptions restees coupees", g_fake.irq_depth, 0);
}

int	main(void)
{
	h_begin("a09/rng_locks");
	h_run("locks/equilibre-verrou-et-interruptions",
		verrous_equilibres_apres_usage_mixte);
	return (h_end());
}
