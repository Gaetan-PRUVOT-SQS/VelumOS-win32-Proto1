#include <stdio.h>
#include <stdlib.h>
#include "velum/err.h"
#include "a02_fake.h"

static void	model_session(uint64_t seed, uint64_t ops)
{
	t_run		*r;
	t_snap		snap;

	h_eq_i64("boot", fake_model_machine(), 0);
	h_eq_i64("modele", model_init(), 0);
	fake_seed(seed);
	printf("modele de reference : graine = %#llx, %llu operations\n",
		(unsigned long long)fake_seed_value(), (unsigned long long)ops);
	fake_snap_take(&snap);
	r = calloc(1, sizeof(*r));
	fake_model_run(r, ops);
	fake_model_drain(r);
	printf("  allocations %llu (refusees %llu), liberations %llu\n",
		(unsigned long long)r->allocs, (unsigned long long)r->refused,
		(unsigned long long)r->frees);
	h_eq_u64("ecarts avec le modele", r->errors, 0);
	h_true(r->refused > 0, "des refus ont ete exerces");
	h_true(fake_snap_same(&snap), "retour a l'etat initial");
	free(r);
	fake_snap_drop(&snap);
	model_done();
}

static void	model_100000_operations(void)
{
	model_session(FAKE_DEFAULT_SEED, 100000);
}

static void	model_other_seeds(void)
{
	model_session(0x0123456789abcdefull, 20000);
	model_session(0xfedcba9876543210ull, 20000);
}

static void	model_oracle_is_not_vacuous(void)
{
	t_mreq	rq;

	h_eq_i64("boot", fake_model_machine(), 0);
	h_eq_i64("modele", model_init(), 0);
	h_eq_u64("modele = implementation au depart", model_diff(), 0);
	pmm_alloc(PMM_KERNEL);
	h_true(model_diff() == 1, "une allocation non enregistree est vue");
	g_pmm.owner[300] = PMM_HEAP;
	h_true(model_diff() == 2, "un proprietaire altere est vu");
	g_pmm.owner[300] = PMM_FREE;
	rq.n = 3000;
	rq.al = 1;
	rq.max = 0;
	h_eq_i64("demande irrealisable", model_feasible(&rq), 0);
	rq.n = 1;
	rq.max = 0xfff;
	h_eq_i64("max sous une page", model_feasible(&rq), 0);
	model_done();
}

int	main(void)
{
	h_begin("a02/model");
	h_run("modele/aleatoire : 100000 operations", model_100000_operations);
	h_run("modele/aleatoire : deux autres graines", model_other_seeds);
	h_run("modele/force de l'oracle", model_oracle_is_not_vacuous);
	return (h_end());
}
