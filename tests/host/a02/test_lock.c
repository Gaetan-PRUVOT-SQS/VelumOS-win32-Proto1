#include "velum/err.h"
#include "a02_fake.h"

static uint32_t	ticket_since(uint32_t t0)
{
	h_eq_u64("verrou rendu", g_pmm.serving, g_pmm.ticket);
	h_eq_i64("interruptions restaurees", fake_irq_depth(), 0);
	return (g_pmm.ticket - t0);
}

static void	lock_ticket_counts(void)
{
	uint32_t	t0;
	uint64_t	phys;
	t_pmm_stats	s;

	h_eq_i64("boot", fake_region_machine(), 0);
	t0 = g_pmm.ticket;
	phys = pmm_alloc(PMM_USER);
	h_eq_u64("alloc : un verrou", ticket_since(t0), 1);
	pmm_free(phys, PMM_USER);
	h_eq_u64("free : un verrou", ticket_since(t0), 2);
	pmm_get_stats(&s);
	pmm_check();
	pmm_fail_after(-1);
	h_eq_u64("stats, check, fail_after", ticket_since(t0), 5);
	pmm_free_pages(0x123, 0, PMM_USER);
	pmm_add_region(0, 0);
	h_eq_u64("free 0 page et region invalide : aucun", ticket_since(t0), 5);
	pmm_add_region(17 * MIB, 2 * MIB);
	h_eq_u64("region valide : un verrou", ticket_since(t0), 6);
}

static void	lock_after_fault(void)
{
	uint32_t	t0;
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	t0 = g_pmm.ticket;
	h_eq_i64("faute", fake_free_try(phys, 1, PMM_KERNEL), 1);
	h_eq_u64("un seul verrou, rendu avant la panique", ticket_since(t0), 1);
	h_eq_i64("faute d'alignement", fake_free_try(phys + 3, 1, PMM_USER), 1);
	h_eq_u64("encore un", ticket_since(t0), 2);
	pmm_free(phys, PMM_USER);
	h_eq_u64("le verrou reste utilisable", ticket_since(t0), 3);
}

int	main(void)
{
	h_begin("a02/lock");
	h_run("verrou/table : un ticket par appel verrouille", lock_ticket_counts);
	h_run("verrou/etat : rendu avant la panique", lock_after_fault);
	return (h_end());
}
