#include <stdint.h>
#include "a04_fake.h"

static void	check_detects_cache_counters(void)
{
	t_cache	*k;
	void	*p;

	a04_fresh();
	p = kmalloc(16);
	k = &g_heap.cls[0].cache[0];
	k->allocs++;
	a04_expect_found("E10 objets du cache");
	k->allocs--;
	k->bytes += 16;
	a04_expect_found("E10 octets du cache");
	k->bytes -= 16;
	k->slabs++;
	a04_expect_found("E10 dalles du cache");
	k->slabs--;
	k->empty++;
	a04_expect_found("E10 dalles vides du cache");
	k->empty--;
	a04_expect_clean("E10 compteurs restaures");
	kfree(p);
	a04_drain("E10 compteurs du cache");
}

static void	check_detects_arena_state(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(16);
	g_heap.arena[0].taken++;
	a04_expect_found("E10 compteur d'arene");
	g_heap.arena[0].taken--;
	g_heap.arena[0].bits[0] |= 1ull << 40;
	a04_expect_found("E10 bit d'arene sans propriétaire");
	g_heap.arena[0].bits[0] &= ~(1ull << 40);
	a04_expect_clean("E10 arene restauree");
	kfree(p);
	a04_drain("E10 arene");
}

static void	check_detects_large_blocks(void)
{
	t_large	*h;
	void	*p;

	a04_fresh();
	p = kmalloc(9000);
	h = (t_large *)((uintptr_t)p - 64);
	h->cookie ^= 1;
	a04_expect_found("E10 cookie de bloc");
	h->cookie ^= 1;
	h->tag = 99;
	a04_expect_found("E10 etiquette de bloc");
	h->tag = 0;
	h->npages++;
	a04_expect_found("E10 pages du bloc");
	h->npages--;
	g_heap.large.allocs[0]++;
	a04_expect_found("E10 compteur des gros blocs");
	g_heap.large.allocs[0]--;
	a04_expect_clean("E10 gros bloc restaure");
	kfree(p);
	a04_drain("E10 gros blocs");
}

static void	check_detects_head_bitmap(void)
{
	t_large	*h;
	void	*p;
	void	*q;

	a04_fresh();
	p = kmalloc(9000);
	q = kmalloc(9000);
	h = (t_large *)((uintptr_t)q - 64);
	large_head_set(arena_index(&g_heap.arena[3], (uintptr_t)h), 0);
	a04_expect_found("E10 bit de tete efface");
	large_head_set(arena_index(&g_heap.arena[3], (uintptr_t)h), 1);
	h->next = NULL;
	a04_expect_found("E10 bloc retire de la liste");
	h->next = (t_large *)((uintptr_t)p - 64);
	h->next->prev = (t_large *)8;
	a04_expect_found("E10 prev d'un gros bloc");
	h->next->prev = h;
	a04_expect_clean("E10 tetes restaurees");
	kfree(p);
	kfree(q);
	a04_drain("E10 bitmap de tetes");
}

int	main(void)
{
	h_begin("a04/heap_check-etat");
	h_run("E10 compteurs du cache", check_detects_cache_counters);
	h_run("E10 etat de l'arene", check_detects_arena_state);
	h_run("E10 gros blocs", check_detects_large_blocks);
	h_run("E10 bitmap de tetes", check_detects_head_bitmap);
	return (h_end());
}
