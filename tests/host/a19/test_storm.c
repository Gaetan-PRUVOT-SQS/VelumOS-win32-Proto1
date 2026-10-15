#include <stdio.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	run(uint32_t seed, int steps)
{
	t_fakeui	u;
	int			i;
	bool		ok;

	printf("a19/storm graine=%u pas=%d\n", seed, steps);
	fake_begin(&u, 320, 260);
	fake_dialog(&u);
	fake_seed(seed);
	ok = true;
	i = 0;
	while (i < steps && ok)
	{
		fake_storm_step(&u);
		ok = fake_storm_ok(&u);
		i++;
	}
	h_true(ok, "arbre coherent apres chaque pas");
	h_eq_i64("tous les pas joues", i, steps);
	ctl_paint(&u.r);
	fake_done(&u);
}

static void	storm_a(void)
{
	run(1u, 40000);
}

static void	storm_b(void)
{
	run(20261005u, 40000);
}

static void	storm_c(void)
{
	run(0xc0ffeeu, 40000);
}

int	main(void)
{
	h_begin("a19/storm");
	h_run("tempete d'evenements et de proprietes, graine 1", storm_a);
	h_run("tempete, graine 20261005", storm_b);
	h_run("tempete, graine 0xc0ffee", storm_c);
	return (h_end());
}
