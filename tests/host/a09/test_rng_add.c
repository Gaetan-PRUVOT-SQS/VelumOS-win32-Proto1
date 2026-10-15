#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

static void	add_vide_ou_nul_sans_effet(void)
{
	t_rng	r;

	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_add(&r, NULL, 100);
	rng_add(&r, "x", 0);
	h_eq_u64("aucun octet compte", r.pool_bytes, 0);
	rng_add(&r, "x", 1);
	h_eq_u64("un octet compte", r.pool_bytes, 1);
}

static void	add_decoupe_aux_frontieres_de_128(void)
{
	static const size_t	sizes[5] = {127, 128, 129, 256, 1000};
	uint8_t				data[1000];
	t_rng				r;
	size_t				i;
	int					bad;

	fake_reset();
	fill_pattern(data, sizeof(data), 777);
	bad = 0;
	i = 0;
	while (i < 5)
	{
		rng_init(&r, &g_rng_env);
		rng_add(&r, data, sizes[i]);
		bad += r.pool_bytes != sizes[i];
		i++;
	}
	h_eq_i64("tailles mal comptees", bad, 0);
}

static void	add_en_un_ou_deux_appels_meme_resultat(void)
{
	uint8_t	data[256];
	t_rng	a;
	t_rng	b;

	fill_pattern(data, sizeof(data), 8032);
	fake_reset();
	rng_init(&a, &g_rng_env);
	rng_add(&a, data, 256);
	rng_reseed(&a);
	fake_reset();
	rng_init(&b, &g_rng_env);
	rng_add(&b, data, 100);
	rng_add(&b, data + 100, 156);
	rng_reseed(&b);
	h_true(!memcmp(a.key, b.key, sizeof(a.key)), "decoupage sans effet");
}

static void	add_change_la_cle_apres_regraine(void)
{
	t_rng	a;
	t_rng	b;

	fake_reset();
	rng_init(&a, &g_rng_env);
	rng_reseed(&a);
	fake_reset();
	rng_init(&b, &g_rng_env);
	rng_add(&b, "evenement", 9);
	rng_reseed(&b);
	h_true(memcmp(a.key, b.key, sizeof(a.key)) != 0, "entropie sans effet");
}

int	main(void)
{
	h_begin("a09/rng_add");
	h_run("add/pointeur-nul-et-taille-nulle", add_vide_ou_nul_sans_effet);
	h_run("add/frontieres-de-128-octets", add_decoupe_aux_frontieres_de_128);
	h_run("add/metamorphique-un-ou-deux-appels",
		add_en_un_ou_deux_appels_meme_resultat);
	h_run("add/entropie-ajoutee-change-la-cle",
		add_change_la_cle_apres_regraine);
	return (h_end());
}
