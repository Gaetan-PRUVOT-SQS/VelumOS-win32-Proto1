#include <stdint.h>
#include "a04_fake.h"

static const uint32_t	g_sizes[] = {20, 60, 200, 700, 1500, 5000, 70000};

static uint64_t	fill_tag(void **row, uint32_t t)
{
	uint64_t	sum;
	uint32_t	k;

	sum = 0;
	k = 0;
	while (k < 8)
	{
		row[k] = kmalloc_tag(g_sizes[(t + k) % 7], (t_heap_tag)t);
		sum += a04_slot(g_sizes[(t + k) % 7]);
		k++;
	}
	return (sum);
}

static void	stats_per_tag_exact(void)
{
	t_heap_stats	st;
	void			*p[HEAP_TAGS][8];
	uint32_t		t;
	uint64_t		sum;

	a04_fresh();
	t = 0;
	while (t < HEAP_TAGS)
	{
		sum = fill_tag(p[t], t);
		heap_get_stats(&st);
		h_eq_u64("E4 octets de l'etiquette", st.bytes_live[t], sum);
		h_eq_u64("E4 objets de l'etiquette", st.allocs_live[t], 8);
		t++;
	}
	t = 0;
	while (t < HEAP_TAGS)
		a04_release(p[t++], 8);
	a04_drain("E4 etiquettes");
}

static void	stats_call_counters(void)
{
	t_heap_stats	st;
	void			*p[10];

	a04_fresh();
	h_eq_u64("E4 dix allocations", a04_grab(100, 0, p, 10), 10);
	heap_fail_after(0);
	h_true(kmalloc(8) == NULL, "E4 echec injecte");
	heap_fail_after(-1);
	heap_get_stats(&st);
	h_eq_u64("E4 appels reussis", st.alloc_calls, 10);
	h_eq_u64("E4 appels echoues", st.fail_calls, 1);
	h_eq_u64("E4 liberations", st.free_calls, 0);
	a04_release(p + 6, 4);
	heap_get_stats(&st);
	h_eq_u64("E4 liberations comptees", st.free_calls, 4);
	h_eq_u64("E4 alloc - free = objets", st.alloc_calls - st.free_calls,
		a04_objects());
	a04_release(p, 6);
	a04_drain("E4 appels");
}

static void	stats_pages_match_backend(void)
{
	t_heap_stats	st;
	void			*p[20];
	size_t			i;

	a04_fresh();
	i = 0;
	while (i < 20)
	{
		p[i] = kmalloc(g_sizes[i % 7]);
		heap_get_stats(&st);
		h_eq_u64("E4 pages du tas = pages du vmm", st.pages_mapped,
			g_fake.mapped);
		i++;
	}
	while (i > 0)
		kfree(p[--i]);
	heap_get_stats(&st);
	h_eq_u64("E4 pages apres liberation", st.pages_mapped, g_fake.mapped);
	a04_drain("E4 pages");
}

int	main(void)
{
	h_begin("a04/statistiques");
	h_run("E4 compteurs par etiquette", stats_per_tag_exact);
	h_run("E4 compteurs d'appels", stats_call_counters);
	h_run("E4 pages et vmm", stats_pages_match_backend);
	return (h_end());
}
