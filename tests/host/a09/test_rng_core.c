#include <string.h>
#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

static void	premiere_lecture_amorce(void)
{
	t_rng	r;
	uint8_t	b;

	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_read(&r, NULL, 8);
	rng_read(&r, &b, 0);
	h_eq_u64("tampon nul ou taille 0 : pas d'amorcage", r.reseeds, 0);
	rng_read(&r, &b, 1);
	h_eq_u64("amorce a la premiere lecture", r.reseeds, 1);
	h_eq_u64("etat amorce", r.seeded, 1);
}

static void	flux_deterministe_et_decoupage_par_blocs_de_32(void)
{
	t_rng	r;
	uint8_t	whole[64];
	uint8_t	parts[64];

	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_read(&r, whole, 64);
	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_read(&r, parts, 32);
	rng_read(&r, parts + 32, 32);
	h_true(!memcmp(whole, parts, 64), "lire 64 = lire 32 puis 32");
	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_read(&r, parts, 32);
	rng_read(&r, parts + 32, 32);
	h_true(!memcmp(whole, parts, 64), "rejeu identique a etat identique");
}

static void	cle_remplacee_a_chaque_bloc(void)
{
	t_rng	r;
	uint8_t	before[CHACHA20_KEY_LEN];
	uint8_t	out[32];
	int		same;
	int		i;

	fake_reset();
	rng_init(&r, &g_rng_env);
	same = 0;
	i = 0;
	while (i < 8)
	{
		rng_read(&r, out, sizeof(out));
		memcpy(before, r.key, sizeof(before));
		rng_read(&r, out, sizeof(out));
		same += !memcmp(before, r.key, sizeof(before));
		same += !memcmp(out, r.key, sizeof(out));
		i++;
	}
	h_eq_i64("cle inchangee ou egale a la sortie", same, 0);
}

static void	tailles_limites_sans_depassement(void)
{
	static const size_t	sizes[9] = {0, 1, 31, 32, 33, 63, 64, 65, 1000};
	uint8_t				buf[1016];
	t_rng				r;
	size_t				i;
	int					bad;

	fake_reset();
	rng_init(&r, &g_rng_env);
	bad = 0;
	i = 0;
	while (i < 9)
	{
		memset(buf, 0xa5, sizeof(buf));
		rng_read(&r, buf, sizes[i]);
		bad += buf[sizes[i]] != 0xa5 || buf[sizes[i] + 7] != 0xa5;
		i++;
	}
	h_eq_i64("octets ecrits au-dela de la taille demandee", bad, 0);
}

int	main(void)
{
	h_begin("a09/rng_core");
	h_run("core/amorcage-paresseux", premiere_lecture_amorce);
	h_run("core/flux-deterministe-et-blocs-de-32",
		flux_deterministe_et_decoupage_par_blocs_de_32);
	h_run("core/effacement-rapide-de-la-cle", cle_remplacee_a_chaque_bloc);
	h_run("core/tailles-0-1-31-32-33-63-64-65-1000",
		tailles_limites_sans_depassement);
	return (h_end());
}
