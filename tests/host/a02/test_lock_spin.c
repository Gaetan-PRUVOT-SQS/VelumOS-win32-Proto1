#include "velum/err.h"
#include "a02_fake.h"

static void	spin_waits_for_holder(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	g_pmm.ticket = g_pmm.serving + 1;
	fake_relax_arm();
	phys = pmm_alloc(PMM_KERNEL);
	h_true(phys != 0, "l'allocation aboutit apres l'attente");
	h_eq_i64("une seule pause", fake_relax_calls(), 1);
	h_eq_u64("verrou rendu", g_pmm.serving, g_pmm.ticket);
	h_eq_i64("interruptions restaurees", fake_irq_depth(), 0);
}

static void	spin_free_path(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_KERNEL);
	g_pmm.ticket = g_pmm.serving + 1;
	fake_relax_arm();
	pmm_free(phys, PMM_KERNEL);
	h_eq_i64("pause pendant la liberation", fake_relax_calls(), 1);
	h_eq_u64("rendu", g_pmm.serving, g_pmm.ticket);
	h_eq_u64("liberee", fake_owned(PMM_KERNEL), 0);
}

int	main(void)
{
	h_begin("a02/lock_spin");
	h_run("verrou/etat : attente d'un detenteur (alloc)",
		spin_waits_for_holder);
	h_run("verrou/etat : attente d'un detenteur (free)", spin_free_path);
	return (h_end());
}
