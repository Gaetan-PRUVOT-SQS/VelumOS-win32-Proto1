#include <string.h>
#include "velum/err.h"
#include "a02_fake.h"

static void	fault_double_free(void)
{
	t_pmm_stats	a;
	t_pmm_stats	b;
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	pmm_free(phys, PMM_USER);
	pmm_get_stats(&a);
	h_eq_i64("double liberation detectee", fake_free_try(phys, 1, PMM_USER), 1);
	h_eq_str("message", fake_assert_msg(), "pmm: double libération");
	h_true(strstr(fake_last_log(), "phys=0x") != NULL, "journal : adresse");
	h_true(strstr(fake_last_log(), "appelant=") != NULL, "journal : appelant");
	pmm_get_stats(&b);
	h_eq_u64("etat inchange", b.free_calls, a.free_calls);
	h_eq_i64("verrou rendu", fake_irq_depth(), 0);
	h_eq_i64("invariants", pmm_check(), 0);
}

static void	fault_wrong_owner(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	h_eq_i64("mauvais proprietaire detecte",
		fake_free_try(phys, 1, PMM_KERNEL), 1);
	h_eq_str("message", fake_assert_msg(), "pmm: mauvais propriétaire");
	h_eq_u64("toujours a l'utilisateur", fake_owned(PMM_USER), 1);
	h_eq_i64("proprietaire PMM_FREE refuse", fake_free_try(phys, 1, PMM_FREE),
		1);
	h_eq_i64("proprietaire hors enum refuse",
		fake_free_try(phys, 1, (t_pmm_owner)PMM_OWNERS), 1);
	h_eq_i64("bon proprietaire accepte", fake_free_try(phys, 1, PMM_USER), 0);
	h_eq_i64("verrou rendu", fake_irq_depth(), 0);
}

static void	fault_address(void)
{
	uint64_t	phys;

	h_eq_i64("boot", fake_simple(1, 8), 0);
	phys = pmm_alloc(PMM_USER);
	h_eq_i64("adresse non alignee", fake_free_try(phys + 1, 1, PMM_USER), 1);
	h_eq_str("message", fake_assert_msg(), "pmm: adresse non alignée");
	h_eq_i64("juste apres le span", fake_free_try(g_pmm.span * PAGE_SIZE, 1,
			PMM_USER), 1);
	h_eq_str("message", fake_assert_msg(),
		"pmm: adresse hors de la mémoire gérée");
	h_eq_i64("derniere page 64 bits", fake_free_try(UINT64_MAX - 0xfff, 1,
			PMM_USER), 1);
	h_eq_i64("n qui deborde", fake_free_try(phys, UINT64_MAX, PMM_USER), 1);
	h_eq_i64("bloc a cheval sur la fin du span", fake_free_try(
			(g_pmm.span - 1) * PAGE_SIZE, 2, PMM_USER), 1);
	h_eq_str("message", fake_assert_msg(),
		"pmm: adresse hors de la mémoire gérée");
	h_eq_u64("toujours alloue", fake_owned(PMM_USER), 1);
}

static void	fault_before_init(void)
{
	fake_reset();
	h_eq_i64("liberation avant pmm_boot_init", fake_free_try(MIB, 1,
			PMM_KERNEL), 1);
	h_eq_str("message", fake_assert_msg(),
		"pmm: adresse hors de la mémoire gérée");
	h_eq_u64("alloc avant init : echec propre", pmm_alloc(PMM_KERNEL), 0);
	h_eq_i64("check avant init", pmm_check(), 0);
}

int	main(void)
{
	h_begin("a02/free_faults");
	h_run("free/etat : double liberation", fault_double_free);
	h_run("free/etat : mauvais proprietaire", fault_wrong_owner);
	h_run("free/limite : adresses invalides", fault_address);
	h_run("free/supposition : avant l'init", fault_before_init);
	return (h_end());
}
