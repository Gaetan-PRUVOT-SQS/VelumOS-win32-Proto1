#include <stdio.h>
#include "harness.h"
#include "fake.h"
#include "ctl_int.h"

static void	run(uint32_t seed, int steps, uint32_t flags, int width)
{
	t_fakeui	u;
	t_ctl		*e;
	int			i;
	bool		ok;

	printf("a19/edit_fuzz graine=%u pas=%d\n", seed, steps);
	fake_begin(&u, 200, 40);
	e = fake_edit(&u, "");
	ctl_set_flag(&u.r, e, flags, true);
	ctl_set_rect(&u.r, e, rect_make(5, 5, width, 20));
	fake_seed(seed);
	ok = true;
	i = 0;
	while (i < steps && ok)
	{
		fake_fuzz_step(&u, e);
		ok = fake_fuzz_ok(e);
		i++;
	}
	h_true(ok, "invariants conserves jusqu'au bout");
	h_eq_i64("iterations jouees", i, steps);
	ctl_paint(&u.r);
	fake_done(&u);
}

static void	fuzz_normal(void)
{
	run(20261005u, 100000, 0, 150);
}

static void	fuzz_password_narrow(void)
{
	run(7u, 50000, CTL_PASSWORD, 24);
}

static void	fuzz_other_seed(void)
{
	run(0xdeadbeefu, 50000, 0, 60);
}

int	main(void)
{
	h_begin("a19/edit_fuzz");
	h_run("100000 operations aleatoires a graine fixe", fuzz_normal);
	h_run("mot de passe, champ etroit", fuzz_password_narrow);
	h_run("autre graine, champ moyen", fuzz_other_seed);
	return (h_end());
}
