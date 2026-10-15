#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

static void	nouvelles_tentatives_materielles(void)
{
	uint8_t	seed[RNG_SEED_LEN];

	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_FLAKY;
	g_fake.rdseed.flaky_fails = 9;
	g_fake.rdseed.pending = 9;
	h_true(rng_gather(&g_rng_env, seed) & RNG_SRC_RDSEED,
		"9 echecs puis succes");
	h_eq_u64("appels rdseed (4 mots x 10 essais)", g_fake.rdseed.calls, 40);
	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_BROKEN;
	h_true(!(rng_gather(&g_rng_env, seed) & RNG_SRC_RDSEED),
		"toujours en echec");
	h_eq_u64("appels rdseed bornes a 10 par mot", g_fake.rdseed.calls, 40);
	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_ABSENT;
	rng_gather(&g_rng_env, seed);
	h_eq_u64("absent : un seul essai par mot", g_fake.rdseed.calls, 4);
}

static void	un_mot_perdu_n_annule_pas_les_suivants(void)
{
	uint8_t	seed[RNG_SEED_LEN];

	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_FLAKY;
	g_fake.rdseed.flaky_fails = 10;
	g_fake.rdseed.pending = 10;
	h_true(rng_gather(&g_rng_env, seed) & RNG_SRC_RDSEED,
		"mots 2 et 4 obtenus malgre les mots 1 et 3 perdus");
	h_eq_u64("appels rdseed : 10 + 1 + 10 + 1", g_fake.rdseed.calls, 22);
}

static void	gigue_nombre_d_echantillons_et_sensibilite(void)
{
	uint8_t	a[RNG_SEED_LEN];
	uint8_t	b[RNG_SEED_LEN];

	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_ABSENT;
	rng_gather(&g_rng_env, a);
	h_eq_u64("echantillons de gigue", g_fake.cycles_calls, RNG_JITTER_SAMPLES);
	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_ABSENT;
	rng_gather(&g_rng_env, b);
	h_true(!memcmp(a, b, sizeof(a)), "meme etat factice, meme graine");
	fake_reset();
	g_fake.rdrand.mode = HW_ABSENT;
	g_fake.rdseed.mode = HW_ABSENT;
	g_fake.now_ns = 123456789;
	rng_gather(&g_rng_env, b);
	h_true(memcmp(a, b, sizeof(a)) != 0, "l'horloge change la graine");
}

int	main(void)
{
	h_begin("a09/rng_retry");
	h_run("gather/nouvelles-tentatives-9-10-et-absent",
		nouvelles_tentatives_materielles);
	h_run("gather/mot-perdu-sans-effet-sur-les-suivants",
		un_mot_perdu_n_annule_pas_les_suivants);
	h_run("gather/gigue-128-echantillons-et-sensibilite",
		gigue_nombre_d_echantillons_et_sensibilite);
	return (h_end());
}
