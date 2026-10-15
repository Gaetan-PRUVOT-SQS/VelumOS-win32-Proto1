#include <stdint.h>
#include "a04_fake.h"

static void	exhaust_one(size_t size, uint32_t expect, const char *what)
{
	void		*hold[A04_HOLD];
	uint32_t	n;

	n = a04_grab(size, 0, hold, A04_HOLD);
	h_eq_u64(what, n, expect);
	a04_release(hold, n);
}

static void	exhaust_each_slab_arena(void)
{
	a04_fresh_with(14, 8);
	exhaust_one(16, 4 * 252, "E8 arene 4 Kio : 4 dalles de 252");
	exhaust_one(400, 2 * 15, "E8 arene 8 Kio : 2 dalles de 15");
	exhaust_one(800, 15, "E8 arene 16 Kio : 1 dalle de 15");
	exhaust_one(16, 4 * 252, "E8 de nouveau 4 Kio apres liberation");
	a04_drain("E8 arenes de dalles");
}

static void	exhaust_reports_failures_without_panic(void)
{
	void			*hold[A04_HOLD];
	t_heap_stats	st;
	uint32_t		n;

	a04_fresh_with(14, 8);
	n = a04_grab(16, 0, hold, A04_HOLD);
	heap_get_stats(&st);
	h_eq_u64("E8 un echec compte", st.fail_calls, 1);
	h_true(kmalloc(16) == NULL, "E8 toujours NULL");
	h_true(kmalloc_tag(16, HEAP_FS) == NULL, "E8 autre etiquette : NULL");
	h_eq_i64("E8 heap_check intact", heap_check(), 0);
	a04_release(hold, n);
	a04_drain("E8 pas de panique");
}

int	main(void)
{
	h_begin("a04/epuisement");
	h_run("E8 epuisement de chaque arene", exhaust_each_slab_arena);
	h_run("E8 echecs sans panique", exhaust_reports_failures_without_panic);
	return (h_end());
}
