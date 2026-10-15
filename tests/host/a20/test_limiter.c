#include <stdint.h>
#include "harness.h"
#include "a20_test.h"
#include "limiter.h"

static void	limiter_delais_croissants(void)
{
	static const uint32_t	want[] = {0, 1, 2, 4, 8, 16, 30, 30, 30, 30, 30};
	uint32_t				i;

	i = 0;
	while (i < sizeof(want) / sizeof(want[0]))
	{
		h_eq_u64("delai", lim_delay_ns(i), want[i] * NS_SEC);
		i++;
	}
	h_eq_u64("compteur max", lim_delay_ns(UINT32_MAX), 30 * NS_SEC);
}

static void	limiter_suite_d_echecs_et_reste(void)
{
	t_limiters	l;
	uint64_t	now;

	lim_init(&l);
	now = 100 * NS_SEC;
	h_eq_u64("libre au depart", lim_remaining_s(&l, 0, now), 0);
	lim_fail(&l, 0, now);
	h_eq_u64("1 s", lim_remaining_s(&l, 0, now), 1);
	h_eq_u64("0,5 s arrondi", lim_remaining_s(&l, 0, now + NS_SEC / 2), 1);
	h_eq_u64("1 ns restante", lim_remaining_s(&l, 0, now + NS_SEC - 1), 1);
	h_eq_u64("fin du delai", lim_remaining_s(&l, 0, now + NS_SEC), 0);
	lim_fail(&l, 0, now + NS_SEC);
	h_eq_u64("2 s apres le second echec",
		lim_remaining_s(&l, 0, now + NS_SEC), 2);
	h_eq_u64("deux echecs comptes", l.slot[0].failures, 2);
}

static void	limiter_remise_a_zero_et_independance(void)
{
	t_limiters	l;

	lim_init(&l);
	lim_fail(&l, 0, 0);
	lim_fail(&l, 0, 0);
	lim_fail(&l, 3, 0);
	h_eq_u64("compte 0 bloque 2 s", lim_remaining_s(&l, 0, 0), 2);
	h_eq_u64("compte 3 bloque 1 s", lim_remaining_s(&l, 3, 0), 1);
	h_eq_u64("compte 1 libre", lim_remaining_s(&l, 1, 0), 0);
	lim_reset(&l, 0);
	h_eq_u64("remis a zero", lim_remaining_s(&l, 0, 0), 0);
	h_eq_u64("compteur remis a zero", l.slot[0].failures, 0);
	lim_fail(&l, 0, 0);
	h_eq_u64("repart de 1 s", lim_remaining_s(&l, 0, 0), 1);
	h_eq_u64("compte 3 inchange", lim_remaining_s(&l, 3, 0), 1);
}

static void	limiter_debordements_et_indices(void)
{
	t_limiters	l;

	lim_init(&l);
	l.slot[1].failures = LIM_FAILURES_MAX;
	lim_fail(&l, 1, 0);
	h_eq_u64("compteur sature", l.slot[1].failures, LIM_FAILURES_MAX);
	l.slot[2].failures = UINT32_MAX;
	lim_fail(&l, 2, 0);
	h_eq_u64("compteur corrompu ne revient pas a 0", l.slot[2].failures,
		UINT32_MAX);
	h_eq_u64("delai maximal", lim_remaining_s(&l, 2, 0), 30);
	lim_fail(&l, 0, UINT64_MAX - NS_SEC / 2);
	h_eq_u64("addition saturee", l.slot[0].blocked_until_ns, UINT64_MAX);
	h_eq_u64("reste proche de la fin", lim_remaining_s(&l, 0,
			UINT64_MAX - NS_SEC / 2), 1);
	lim_fail(&l, ACC_MAX, 0);
	lim_reset(&l, ACC_MAX + 7);
	h_eq_u64("indice hors bornes", lim_remaining_s(&l, ACC_MAX, 0), 0);
	h_eq_u64("indice max 32 bits", lim_remaining_s(&l, UINT32_MAX, 0), 0);
}

int	main(void)
{
	h_begin("a20/limiter");
	h_run("limiter: delais croissants", limiter_delais_croissants);
	h_run("limiter: suite d'echecs et reste", limiter_suite_d_echecs_et_reste);
	h_run("limiter: remise a zero, independance",
		limiter_remise_a_zero_et_independance);
	h_run("limiter: debordements, indices", limiter_debordements_et_indices);
	return (h_end());
}
