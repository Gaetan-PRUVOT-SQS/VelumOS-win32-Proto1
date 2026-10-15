#include "a09_test.h"
#include "fake.h"
#include "harness.h"
#include "rng_int.h"

#define NS_PER_S 1000000000ull

static void	reseed_sans_horloge(void)
{
	t_rng		r;
	t_rng_env	env;
	uint8_t		b;
	int			i;

	fake_reset();
	env = g_rng_env;
	env.now_ns = NULL;
	rng_init(&r, &env);
	i = 0;
	while (i < 50)
	{
		rng_read(&r, &b, 1);
		i++;
	}
	h_eq_u64("sans horloge : une seule regraine", r.reseeds, 1);
	h_eq_u64("sans horloge : date nulle", r.last_reseed_ns, 0);
}

static void	amorce_avant_l_horloge_puis_horloge_active(void)
{
	t_rng	r;
	uint8_t	b;

	fake_reset();
	rng_init(&r, &g_rng_env);
	rng_read(&r, &b, 1);
	h_eq_u64("amorce avec horloge a zero", r.last_reseed_ns, 0);
	g_fake.now_ns = 299 * NS_PER_S;
	rng_read(&r, &b, 1);
	h_eq_u64("299 s apres l'amorce : rien", r.reseeds, 1);
	g_fake.now_ns = 300 * NS_PER_S;
	rng_read(&r, &b, 1);
	h_eq_u64("300 s apres l'amorce : regraine", r.reseeds, 2);
}

int	main(void)
{
	h_begin("a09/rng_clock");
	h_run("reseed/horloge-absente", reseed_sans_horloge);
	h_run("reseed/amorce-avant-l-horloge",
		amorce_avant_l_horloge_puis_horloge_active);
	return (h_end());
}
