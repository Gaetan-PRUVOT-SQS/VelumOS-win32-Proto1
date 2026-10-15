#include <stdio.h>
#include <string.h>
#include "a04_fake.h"

void	a04_fresh_with(uint32_t slab_shift, uint32_t large_pages)
{
	fake_pages_setup(slab_shift, large_pages);
	memset(&g_heap, 0, sizeof(g_heap));
	h_eq_i64("heap_boot_init", heap_boot_init(), 0);
}

void	a04_fresh(void)
{
	a04_fresh_with(A04_SHIFT, A04_LARGE);
}

static void	drain_tags(const t_heap_stats *st)
{
	uint32_t	t;

	t = 0;
	while (t < HEAP_TAGS)
	{
		h_eq_u64("fin: octets vivants par etiquette", st->bytes_live[t], 0);
		h_eq_u64("fin: objets vivants par etiquette", st->allocs_live[t], 0);
		t++;
	}
}

void	a04_drain(const char *what)
{
	t_heap_stats	st;
	uint32_t		i;

	h_eq_i64(what, heap_check(), 0);
	heap_trim();
	heap_get_stats(&st);
	drain_tags(&st);
	h_eq_u64("fin: pages du tas", st.pages_mapped, 0);
	h_eq_u64("fin: pages du faux vmm", g_fake.mapped, 0);
	h_eq_u64("fin: violations du faux vmm", g_fake.violations, 0);
	h_eq_u64("fin: alloc - free", st.alloc_calls, st.free_calls);
	i = 0;
	while (i < HEAP_ARENAS)
	{
		h_eq_u64("fin: arene vide", g_heap.arena[i].taken, 0);
		i++;
	}
	h_eq_i64("fin: verrous equilibres", fake_irq_depth(), 0);
	h_eq_i64("fin: heap_check", heap_check(), 0);
}

void	a04_expect_panic(void (*fn)(void *), void *arg, const char *want)
{
	int	raised;

	raised = fake_catch(fn, arg);
	h_true(raised, "panique attendue");
	if (raised)
	{
		h_true(strstr(fake_panic_msg(), want) != NULL, want);
		if (strstr(fake_panic_msg(), want) == NULL)
			fprintf(stderr, "  message : %s\n", fake_panic_msg());
	}
}
