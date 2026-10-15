#include "velum/err.h"
#include "a02_fake.h"

static const char	*g_modes[] = {"double", "owner", "align", "range"};
static const char	*g_msgs[] = {"pmm: double libération",
	"pmm: mauvais propriétaire", "pmm: adresse non alignée",
	"pmm: adresse hors de la mémoire gérée"};

static void	run_selftest(void *arg)
{
	(void)arg;
	pmm_selftest();
}

static void	fault_modes_panic(void)
{
	size_t	i;

	i = 0;
	while (i < 4)
	{
		h_eq_i64("boot", fake_simple(1, 64), 0);
		fake_cmdline(g_modes[i]);
		h_eq_i64(g_modes[i], fake_catch(run_selftest, NULL), 1);
		h_eq_str("message", fake_assert_msg(), g_msgs[i]);
		h_eq_i64("verrou rendu", fake_irq_depth(), 0);
		i++;
	}
	fake_cmdline(NULL);
}

static void	fault_unknown_mode(void)
{
	h_eq_i64("boot", fake_simple(1, 64), 0);
	fake_cmdline("inconnu");
	h_eq_i64("pas de panique", fake_catch(run_selftest, NULL), 0);
	h_eq_i64("autotest propre", pmm_check(), 0);
	fake_cmdline(NULL);
}

int	main(void)
{
	h_begin("a02/selftest_fault");
	h_run("autotest/etat : fautes provoquees par la ligne de commande",
		fault_modes_panic);
	h_run("autotest/limite : mode inconnu ignore", fault_unknown_mode);
	return (h_end());
}
