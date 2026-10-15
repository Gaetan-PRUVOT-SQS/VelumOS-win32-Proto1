#include <stdint.h>
#include "a04_fake.h"

static void	selftest_passes_on_a_fresh_heap(void)
{
	a04_fresh();
	h_eq_i64("E15 autotest du noyau sur l'hote", heap_selftest(), 0);
	a04_drain("E15 autotest");
}

static void	selftest_is_repeatable_and_leaves_no_trace(void)
{
	t_heap_stats	a;
	t_heap_stats	b;

	a04_fresh();
	h_eq_i64("E15 premier passage", heap_selftest(), 0);
	heap_get_stats(&a);
	h_eq_i64("E15 second passage", heap_selftest(), 0);
	heap_get_stats(&b);
	h_eq_u64("E15 objets vivants identiques", a04_objects(), 0);
	h_eq_u64("E15 pages identiques", b.pages_mapped, a.pages_mapped);
	a04_drain("E15 repetition");
}

static void	selftest_survives_existing_allocations(void)
{
	void	*p[5];
	size_t	i;

	a04_fresh();
	i = 0;
	while (i < 5)
	{
		p[i] = kmalloc_tag(30 + i * 700, (t_heap_tag)i);
		i++;
	}
	h_eq_i64("E15 avec des objets deja vivants", heap_selftest(), 0);
	h_eq_u64("E15 les objets vivants restent", a04_objects(), 5);
	i = 0;
	while (i < 5)
		kfree(p[i++]);
	a04_drain("E15 objets preexistants");
}

int	main(void)
{
	h_begin("a04/autotest");
	h_run("E15 autotest", selftest_passes_on_a_fresh_heap);
	h_run("E15 repetition", selftest_is_repeatable_and_leaves_no_trace);
	h_run("E15 objets preexistants", selftest_survives_existing_allocations);
	return (h_end());
}
