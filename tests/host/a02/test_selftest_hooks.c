#include "velum/err.h"
#include "a02_fake.h"

static void	run_selftest(void *arg)
{
	*(int *)arg = pmm_selftest();
}

static int	detected(void)
{
	int	rc;

	rc = 0;
	if (fake_catch(run_selftest, &rc))
		return (1);
	return (rc != 0);
}

static int	sweep(void (*fn)(void))
{
	int	at;
	int	missed;

	at = 1;
	missed = 0;
	while (at < 400)
	{
		h_eq_i64("boot", fake_simple(1, 16), 0);
		fake_hook_arm(at, fn);
		fake_corrupt_reset();
		if (!detected() && fake_corrupt_took_effect())
			missed++;
		if (!fake_hook_fired())
			break ;
		at++;
	}
	fake_hook_clear();
	h_true(at > 20, "plus de vingt points d'injection");
	return (missed);
}

static void	sweep_all_corruptions(void)
{
	h_eq_i64("compteur de libres corrompu", sweep(fake_corrupt_free), 0);
	h_eq_i64("compteur de proprietaire corrompu", sweep(fake_corrupt_owned), 0);
	h_eq_i64("bit de bitmap corrompu", sweep(fake_corrupt_bit), 0);
	h_eq_i64("injection desarmee : un point equivalent",
		sweep(fake_corrupt_disarm), 1);
	h_eq_i64("proprietaire d'octet corrompu", sweep(fake_corrupt_owner), 0);
}

int	main(void)
{
	h_begin("a02/selftest_hooks");
	h_run("autotest/injection : corruption a chaque appel verrouille",
		sweep_all_corruptions);
	return (h_end());
}
