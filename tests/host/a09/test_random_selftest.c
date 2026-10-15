#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "velum/random.h"

static void	autotest_passe_sur_l_hote(void)
{
	fake_reset();
	g_fake.pmm.total_pages = 1000;
	g_fake.pmm.free_pages = 10;
	h_eq_i64("random_selftest", random_selftest(), 0);
	h_eq_i64("verrous desequilibres", g_fake.lock_errors, 0);
}

static void	autotest_detecte_une_incoherence(void)
{
	fake_reset();
	g_fake.pmm.total_pages = 0;
	h_eq_i64("memoire totale nulle detectee", random_selftest(), 7);
	h_true(fake_log_has("controle des appels systeme 7"), "journal d'echec");
	fake_reset();
	g_fake.ncpus = 0;
	g_fake.pmm.total_pages = 10;
	h_eq_i64("zero processeur detecte", random_selftest(), 6);
}

int	main(void)
{
	h_begin("a09/random_selftest");
	h_run("selftest/passe-avec-des-fakes", autotest_passe_sur_l_hote);
	h_run("selftest/detecte-une-incoherence", autotest_detecte_une_incoherence);
	return (h_end());
}
