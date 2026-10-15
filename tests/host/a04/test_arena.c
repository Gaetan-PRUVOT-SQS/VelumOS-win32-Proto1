#include <stdint.h>
#include "a04_fake.h"

static void	arena_sequential_lowest_first(void)
{
	t_arena		a;
	uint64_t	bits[2];
	uint64_t	i;

	a04_arena_setup(&a, bits, 70);
	i = 0;
	while (i < 70)
		h_eq_u64("E7 ordre croissant", arena_take(&a, 1), i++);
	h_eq_u64("E7 pleine", arena_take(&a, 1), ARENA_NONE);
	h_eq_u64("E7 compteur", a.taken, 70);
	arena_give(&a, 5, 1);
	arena_give(&a, 69, 1);
	arena_give(&a, 0, 1);
	h_eq_u64("E7 le plus bas d'abord", arena_take(&a, 1), 0);
	h_eq_u64("E7 puis le suivant", arena_take(&a, 1), 5);
	h_eq_u64("E7 puis le dernier bit valide", arena_take(&a, 1), 69);
	h_eq_u64("E7 de nouveau pleine", arena_take(&a, 1), ARENA_NONE);
	h_eq_i64("E7 verrou equilibre", fake_irq_depth(), 0);
}

static void	arena_runs_first_fit(void)
{
	t_arena		a;
	uint64_t	bits[4];
	uint64_t	i;

	a04_arena_setup(&a, bits, 200);
	h_eq_u64("E7 run 1", arena_take(&a, 3), 0);
	h_eq_u64("E7 run 2", arena_take(&a, 3), 3);
	h_eq_u64("E7 run 3", arena_take(&a, 3), 6);
	arena_give(&a, 3, 3);
	h_eq_u64("E7 trou trop petit saute", arena_take(&a, 4), 9);
	h_eq_u64("E7 trou exact reutilise", arena_take(&a, 3), 3);
	i = 0;
	while (i < 53)
		h_eq_u64("E7 simples", arena_take(&a, 1), 13 + i++);
	h_eq_u64("E7 run a cheval sur deux mots", arena_take(&a, 5), 66);
	h_eq_u64("E7 run nul", arena_take(&a, 0), ARENA_NONE);
	h_eq_u64("E7 run trop long", arena_take(&a, 201), ARENA_NONE);
}

static void	arena_limits(void)
{
	t_arena		a;
	uint64_t	bits[2];

	a04_arena_setup(&a, bits, 64);
	h_eq_u64("E7 64 d'un coup", arena_take(&a, 64), 0);
	h_eq_u64("E7 plus rien", arena_take(&a, 1), ARENA_NONE);
	arena_give(&a, 0, 64);
	h_eq_u64("E7 tout rendu", a.taken, 0);
	a04_arena_setup(&a, bits, 65);
	h_eq_u64("E7 65 d'un coup", arena_take(&a, 65), 0);
	arena_give(&a, 0, 65);
	h_eq_u64("E7 66 refuse", arena_take(&a, 66), ARENA_NONE);
	h_eq_u64("E7 un seul dans 65", arena_take(&a, 65), 0);
	h_true(arena_test(&a, 64) && !arena_test(&a, 65), "E7 test de bit");
	h_eq_u64("E7 adresse", arena_addr(&a, 3), 0x100000 + 3 * 4096);
	h_eq_u64("E7 index", arena_index(&a, 0x100000 + 7 * 4096 + 5), 7);
}

int	main(void)
{
	h_begin("a04/arene");
	h_run("E7 ordre croissant", arena_sequential_lowest_first);
	h_run("E7 plages contigues premier ajustement", arena_runs_first_fit);
	h_run("E7 bornes", arena_limits);
	return (h_end());
}
