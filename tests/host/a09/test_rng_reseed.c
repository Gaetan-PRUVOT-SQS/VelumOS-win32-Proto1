#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

#define NS_PER_S 1000000000ull

static void	start_at(t_rng *r, uint64_t now_ns)
{
	uint8_t	b;

	fake_reset();
	g_fake.now_ns = now_ns;
	rng_init(r, &g_rng_env);
	rng_read(r, &b, 1);
}

static void	reseed_par_volume_frontieres(void)
{
	t_rng	r;
	uint8_t	b;

	start_at(&r, 0);
	h_eq_u64("amorcage a la premiere lecture", r.reseeds, 1);
	r.since_reseed = RNG_RESEED_BYTES - 1;
	rng_read(&r, &b, 1);
	h_eq_u64("1 Mio - 1 octet : pas de regraine", r.reseeds, 1);
	rng_read(&r, &b, 1);
	h_eq_u64("1 Mio atteint : regraine a la lecture suivante", r.reseeds, 2);
	h_eq_u64("compteur d'octets remis a zero puis incremente",
		r.since_reseed, 1);
}

static void	reseed_par_temps_frontieres(void)
{
	t_rng	r;
	uint8_t	b;

	start_at(&r, 10 * NS_PER_S);
	g_fake.now_ns = 10 * NS_PER_S + 300 * NS_PER_S - 1;
	rng_read(&r, &b, 1);
	h_eq_u64("300 s - 1 ns : pas de regraine", r.reseeds, 1);
	g_fake.now_ns = 10 * NS_PER_S + 300 * NS_PER_S;
	rng_read(&r, &b, 1);
	h_eq_u64("300 s : regraine", r.reseeds, 2);
	h_eq_u64("date de regraine memorisee", r.last_reseed_ns, 310 * NS_PER_S);
	g_fake.now_ns = 5 * NS_PER_S;
	rng_read(&r, &b, 1);
	h_eq_u64("horloge qui recule : rien", r.reseeds, 2);
}

static void	reseed_par_reserve_d_entropie(void)
{
	t_rng	r;
	uint8_t	b;
	uint8_t	pool[64];

	start_at(&r, 10 * NS_PER_S);
	fill_pattern(pool, sizeof(pool), 1);
	g_fake.now_ns = 20 * NS_PER_S;
	rng_add(&r, pool, 63);
	rng_read(&r, &b, 1);
	h_eq_u64("63 octets ajoutes : pas de regraine", r.reseeds, 1);
	rng_add(&r, pool, 1);
	g_fake.now_ns = 10 * NS_PER_S + NS_PER_S - 1;
	rng_read(&r, &b, 1);
	h_eq_u64("64 octets mais < 1 s : pas de regraine", r.reseeds, 1);
	g_fake.now_ns = 10 * NS_PER_S + NS_PER_S;
	rng_read(&r, &b, 1);
	h_eq_u64("64 octets et 1 s : regraine", r.reseeds, 2);
	h_eq_u64("reserve videe apres regraine", r.pool_bytes, 0);
}

int	main(void)
{
	h_begin("a09/rng_reseed");
	h_run("reseed/volume-1-mio-frontieres", reseed_par_volume_frontieres);
	h_run("reseed/temps-300-s-frontieres", reseed_par_temps_frontieres);
	h_run("reseed/reserve-64-octets-et-1-s", reseed_par_reserve_d_entropie);
	return (h_end());
}
