#include <stdint.h>
#include "a04_fake.h"

static void	arena_give_errors(void)
{
	t_arena		a;
	uint64_t	bits[2];
	t_give		g;

	a04_arena_setup(&a, bits, 70);
	h_eq_u64("E7 prise", arena_take(&a, 3), 0);
	g.a = &a;
	g.idx = 0;
	g.n = 4;
	a04_expect_panic(a04_do_give, &g, "plage non réservée");
	g.idx = 1;
	g.n = 3;
	a04_expect_panic(a04_do_give, &g, "plage non réservée");
	g.idx = 69;
	g.n = 2;
	a04_expect_panic(a04_do_give, &g, "plage non réservée");
	g.idx = 0;
	g.n = 0;
	a04_expect_panic(a04_do_give, &g, "plage non réservée");
	g.n = 3;
	a04_do_give(&g);
	a04_expect_panic(a04_do_give, &g, "plage non réservée");
	h_eq_u64("E7 compteur intact apres les refus", a.taken, 0);
	h_eq_i64("E7 verrou equilibre apres les refus", fake_irq_depth(), 0);
}

int	main(void)
{
	h_begin("a04/arene-rendu");
	h_run("E7 rendu invalide", arena_give_errors);
	return (h_end());
}
