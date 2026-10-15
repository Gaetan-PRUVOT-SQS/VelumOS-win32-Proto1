#include <stdint.h>
#include "a04_fake.h"

static void	life_fill_then_drain(void)
{
	void		*p[800];
	uint32_t	n;
	uint32_t	i;
	t_cache		*k;

	a04_fresh();
	k = &g_heap.cls[0].cache[HEAP_GENERIC];
	n = 3 * g_heap.cls[0].nobj + 1;
	i = 0;
	while (i < n)
		p[i++] = kmalloc(16);
	h_eq_u64("E6 quatre dalles", g_fake.mapped, 4);
	h_eq_u64("E6 dalles du cache", k->slabs, 4);
	i = 0;
	while (i < n)
		kfree(p[i++]);
	h_eq_u64("E6 une dalle gardee", g_fake.mapped, 1);
	h_eq_u64("E6 cache : une dalle vide", k->empty, 1);
	h_eq_i64("E6 verification", heap_check(), 0);
	heap_trim();
	h_eq_u64("E6 tout rendu apres trim", g_fake.mapped, 0);
	a04_drain("E6 remplir puis vider");
}

static void	thrash_loop(const char *what)
{
	uint64_t	maps;
	uint32_t	i;

	maps = g_fake.maps;
	i = 0;
	while (i++ < 1000)
		kfree(kmalloc(16));
	h_eq_u64(what, g_fake.maps, maps);
	h_eq_u64("E6 sans va-et-vient : aucun demontage", g_fake.unmaps, 0);
}

static void	life_no_thrash(void)
{
	void		*fill[300];
	uint32_t	i;
	uint32_t	n;

	a04_fresh();
	n = g_heap.cls[0].nobj;
	kfree(kmalloc(16));
	thrash_loop("E6 une dalle : aucun mappage");
	i = 0;
	while (i < n + 1)
		fill[i++] = kmalloc(16);
	kfree(fill[n]);
	thrash_loop("E6 deux dalles : aucun mappage");
	a04_release(fill, n);
	a04_drain("E6 sans va-et-vient");
}

static void	life_partial_slab_is_reused_first(void)
{
	void		*p[260];
	uint32_t	n;
	uint32_t	i;
	void		*q;

	a04_fresh();
	n = g_heap.cls[0].nobj;
	i = 0;
	while (i < n + 1)
	{
		p[i] = kmalloc(16);
		i++;
	}
	kfree(p[10]);
	q = kmalloc(16);
	h_true(q == p[10], "E6 la dalle partielle sert d'abord");
	h_eq_u64("E6 toujours deux dalles", g_fake.mapped, 2);
	i = 0;
	while (i < n + 1)
		kfree(p[i++]);
	a04_drain("E6 dalle partielle");
}

int	main(void)
{
	h_begin("a04/dalles-cycle");
	h_run("E6 remplir puis vider", life_fill_then_drain);
	h_run("E6 pas de va-et-vient", life_no_thrash);
	h_run("E6 dalle partielle reutilisee", life_partial_slab_is_reused_first);
	return (h_end());
}
