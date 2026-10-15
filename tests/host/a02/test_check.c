#include "velum/err.h"
#include "a02_fake.h"

static void	check_clean_states(void)
{
	uint64_t	live[2000];
	uint64_t	nlive;

	fake_reset();
	h_eq_i64("avant l'init", pmm_check(), 0);
	fake_reset();
	fake_range(MIB, 0x41000, MEM_USABLE);
	h_eq_i64("boot, span non multiple de 64", fake_boot(), 0);
	h_eq_u64("span 321", g_pmm.span, 321);
	h_eq_i64("juste apres le boot", pmm_check(), 0);
	nlive = 0;
	fake_seed(7);
	fake_churn(live, &nlive, 40, 3000);
	h_eq_i64("apres du trafic", pmm_check(), 0);
}

static void	check_detects_frame_tampering(void)
{
	h_eq_i64("boot", fake_simple(1, 8), 0);
	g_pmm.bits[5] ^= 1ull << 3;
	h_eq_i64("bit retourne : un ecart", pmm_check(), 1);
	g_pmm.bits[5] ^= 1ull << 3;
	h_eq_i64("restaure", pmm_check(), 0);
	g_pmm.owner[300] = PMM_HEAP;
	h_true(pmm_check() >= 1, "proprietaire sans bit ni compteur");
	g_pmm.owner[300] = 0x77;
	h_true(pmm_check() >= 1, "octet de proprietaire inconnu");
	g_pmm.owner[300] = PMM_FREE;
	h_eq_i64("restaure", pmm_check(), 0);
}

static void	check_detects_counter_tampering(void)
{
	h_eq_i64("boot", fake_simple(1, 8), 0);
	g_pmm.stats.free_pages++;
	h_true(pmm_check() >= 1, "libres faux");
	g_pmm.stats.free_pages--;
	g_pmm.stats.owned[PMM_USER]++;
	h_true(pmm_check() >= 1, "proprietaire faux");
	g_pmm.stats.owned[PMM_USER]--;
	g_pmm.stats.reserved_pages--;
	h_true(pmm_check() >= 1, "reserve faux");
	g_pmm.stats.reserved_pages++;
	h_eq_i64("restaure", pmm_check(), 0);
	g_pmm.bits[g_pmm.words - 1] &= ~(1ull << 63);
	h_true(pmm_check() >= 1, "bit de bourrage efface");
}

int	main(void)
{
	h_begin("a02/check");
	h_run("check/etat : sain", check_clean_states);
	h_run("check/supposition : frames alterees", check_detects_frame_tampering);
	h_run("check/supposition : compteurs alteres",
		check_detects_counter_tampering);
	return (h_end());
}
