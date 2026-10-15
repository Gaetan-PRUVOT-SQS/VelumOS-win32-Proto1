#include <stdint.h>
#include "fake_sys.h"
#include "harness.h"
#include "stdlib.h"

static int	rand_distinct(int *v, int n)
{
	int	i;

	i = 1;
	while (i < n)
	{
		if (v[i] != v[0])
			return (1);
		i++;
	}
	return (0);
}

static void	rand_pool_refill(void)
{
	int	v[16];
	int	i;
	int	bad;

	fake_reset();
	fake_kernel_on();
	bad = 0;
	i = 0;
	while (i < 64)
	{
		v[i % 16] = rand();
		bad += v[i % 16] < 0;
		i++;
	}
	h_eq_u64("un seul appel noyau pour 64 tirages", g_fsys.random_calls, 1);
	rand();
	h_eq_u64("nouveau remplissage au 65e tirage", g_fsys.random_calls, 2);
	h_eq_i64("valeurs dans [0, RAND_MAX]", bad, 0);
	h_true(rand_distinct(v, 16), "valeurs non constantes");
}

static void	rand_getrandom_failure(void)
{
	int	v[200];
	int	i;
	int	bad;

	fake_reset();
	fake_kernel_on();
	g_fsys.random_fail = 1;
	bad = 0;
	i = 0;
	while (i < 200)
	{
		v[i] = rand();
		bad += v[i] < 0;
		i++;
	}
	h_eq_i64("repli : valeurs dans [0, RAND_MAX]", bad, 0);
	h_true(rand_distinct(v, 200), "repli : valeurs non constantes");
	h_true(g_fsys.random_calls >= 2, "le noyau est reinterroge par tampon");
}

int	main(void)
{
	h_begin("a14/rand");
	h_run("rand/etat : remplissage du tampon", rand_pool_refill);
	h_run("rand/injection : SYS_GETRANDOM en panne", rand_getrandom_failure);
	return (h_end());
}
