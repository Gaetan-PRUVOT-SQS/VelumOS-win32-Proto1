#include "harness.h"
#include "fake.h"
#include "time_int.h"
#include "velum/err.h"

static bool	cond_count(void *ctx)
{
	int	*left;

	left = ctx;
	if (!left)
		return (false);
	if (*left > 0)
		(*left)--;
	return (*left == 0);
}

static bool	cond_time(void *ctx)
{
	return (g_fclock.now >= *(uint64_t *)ctx);
}

static void	wait_ready(void)
{
	int			n;
	uint64_t	t0;

	fclock_reset(true, 10);
	h_eq_i64("condition nulle", wait_until(NULL, NULL, 5), E_INVAL);
	n = 1;
	h_eq_i64("vraie tout de suite", wait_until(cond_count, &n, 0), E_OK);
	h_eq_u64("sans attente active", g_fclock.relax, 0);
	n = 5;
	h_eq_i64("vraie apres 5 tours", wait_until(cond_count, &n, 1000), E_OK);
	t0 = g_fclock.now;
	h_eq_i64("jamais vraie", wait_until(cond_count, NULL, 1000), E_TIMEOUT);
	h_true(g_fclock.now - t0 >= 1000, "delai ecoule au retour");
	h_eq_i64("delai nul faux", wait_until(cond_count, NULL, 0), E_TIMEOUT);
	n = 3;
	h_eq_i64("delai sature", wait_until(cond_count, &n, UINT64_MAX), E_OK);
	t0 = g_fclock.now + 1000;
	h_eq_i64("vraie a l'echeance", wait_until(cond_time, &t0, 1000), E_OK);
}

static void	wait_not_ready(void)
{
	int	n;

	fclock_reset(false, 0);
	h_eq_i64("sans horloge jamais vraie", wait_until(cond_count, NULL, 5000),
		E_TIMEOUT);
	h_eq_u64("pas d'une microseconde", g_fclock.delays, 5);
	n = 3;
	h_eq_i64("sans horloge vraie apres 3", wait_until(cond_count, &n, 5000),
		E_OK);
	fclock_reset(false, 0);
	h_eq_i64("sans horloge delai sous 1 us", wait_until(cond_count, NULL, 999),
		E_TIMEOUT);
	h_eq_u64("aucune attente", g_fclock.delays, 0);
}

int	main(void)
{
	h_begin("a05/wait");
	h_run("attente avec horloge", wait_ready);
	h_run("attente avant l'horloge", wait_not_ready);
	return (h_end());
}
