#include "a04_fake.h"

t_slab	*a04_slab_of(const void *p)
{
	uint32_t	arena;

	h_true(heap_locate((uintptr_t)p, &arena) == HEAP_KIND_SLAB, "dalle");
	return (slab_resolve(arena, p, p));
}

void	a04_expect_found(const char *what)
{
	int	found;

	fake_klog_quiet(1);
	found = heap_check();
	fake_klog_quiet(0);
	h_true(found > 0, what);
}

void	a04_expect_clean(const char *what)
{
	h_eq_i64(what, heap_check(), 0);
}
