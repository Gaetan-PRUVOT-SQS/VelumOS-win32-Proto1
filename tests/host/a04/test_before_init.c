#include <stdint.h>
#include <string.h>
#include "a04_fake.h"

static void	before_init_allocation_panics(void)
{
	t_call	call;

	fake_pages_setup(A04_SHIFT, A04_LARGE);
	memset(&g_heap, 0, sizeof(g_heap));
	call.n = 16;
	a04_expect_panic(a04_do_malloc, &call, "avant heap_boot_init");
	a04_expect_panic(a04_do_realloc, &call, "avant heap_boot_init");
	a04_expect_panic(kfree, &call, "avant heap_boot_init");
	kfree(NULL);
	h_true(1, "E14 kfree(NULL) accepte avant init");
}

static void	before_init_queries_are_inert(void)
{
	t_heap_stats	st;

	fake_pages_setup(A04_SHIFT, A04_LARGE);
	memset(&g_heap, 0, sizeof(g_heap));
	st.alloc_calls = 7;
	heap_get_stats(&st);
	h_eq_u64("E14 stats : zero", st.alloc_calls, 0);
	h_eq_i64("E14 heap_check : zero", heap_check(), 0);
	heap_trim();
	heap_fail_after(5);
	h_eq_i64("E14 fail_after memorise", g_heap.fail_after, 5);
	heap_fail_after(-9);
	h_eq_i64("E14 fail_after negatif = desactive", g_heap.fail_after, -1);
}

static void	null_pointers_are_inert(void)
{
	t_heap_stats	st;

	a04_fresh();
	heap_get_stats(NULL);
	kfree(NULL);
	heap_get_stats(&st);
	h_eq_u64("E4 kfree(NULL) sans effet", st.free_calls, 0);
	a04_drain("E4 NULL");
}

int	main(void)
{
	h_begin("a04/avant-init");
	h_run("E14 allocation avant init", before_init_allocation_panics);
	h_run("E14 requetes inertes", before_init_queries_are_inert);
	h_run("E4 NULL", null_pointers_are_inert);
	return (h_end());
}
