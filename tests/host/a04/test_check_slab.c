#include <stdint.h>
#include "a04_fake.h"

static void	check_clean_heap_is_clean(void)
{
	void	*p[4];

	a04_fresh();
	a04_expect_clean("E10 tas vide");
	p[0] = kmalloc(16);
	p[1] = kmalloc_tag(100, HEAP_FS);
	p[2] = kmalloc(2048);
	p[3] = kmalloc(70000);
	a04_expect_clean("E10 tas sain");
	kfree(p[1]);
	a04_expect_clean("E10 apres une liberation");
	kfree(p[0]);
	kfree(p[2]);
	kfree(p[3]);
	a04_drain("E10 sain");
}

static void	check_detects_counter_and_bitmap(void)
{
	t_slab	*s;
	void	*p;

	a04_fresh();
	p = kmalloc(16);
	s = a04_slab_of(p);
	s->inuse++;
	a04_expect_found("E10 inuse trop grand");
	s->inuse--;
	slab_bits(s)[0] &= ~1ull;
	a04_expect_found("E10 bit vivant efface");
	slab_bits(s)[0] |= 1ull;
	slab_bits(s)[0] |= 1ull << 9;
	a04_expect_found("E10 bit libre allume");
	slab_bits(s)[0] &= ~(1ull << 9);
	slab_bits(s)[3] &= ~(1ull << 63);
	a04_expect_found("E10 bit de remplissage efface");
	slab_bits(s)[3] |= 1ull << 63;
	a04_expect_clean("E10 etat restaure");
	kfree(p);
	a04_drain("E10 compteur et bitmap");
}

static void	check_detects_header_tamper(void)
{
	t_slab	*s;
	void	*p;

	a04_fresh();
	p = kmalloc(16);
	s = a04_slab_of(p);
	s->magic ^= 1;
	a04_expect_found("E10 cookie de dalle");
	s->magic ^= 1;
	s->nobj++;
	a04_expect_found("E10 nombre d'objets");
	s->nobj--;
	s->tag ^= 1;
	a04_expect_found("E10 etiquette de la dalle");
	s->tag ^= 1;
	s->full = 1;
	a04_expect_found("E10 drapeau plein");
	s->full = 0;
	a04_expect_clean("E10 etat restaure");
	kfree(p);
	a04_drain("E10 en-tete");
}

static void	check_detects_list_tamper(void)
{
	t_slab	*s;
	t_cache	*k;
	void	*p;

	a04_fresh();
	p = kmalloc(16);
	s = a04_slab_of(p);
	k = &g_heap.cls[0].cache[0];
	s->prev = (t_slab *)s;
	a04_expect_found("E10 prev invalide");
	s->prev = NULL;
	s->next = s;
	a04_expect_found("E10 cycle de liste");
	s->next = NULL;
	k->partial.tail = NULL;
	a04_expect_found("E10 queue de liste");
	k->partial.tail = s;
	a04_expect_clean("E10 listes restaurees");
	kfree(p);
	a04_drain("E10 listes");
}

int	main(void)
{
	h_begin("a04/heap_check-dalles");
	h_run("E10 tas sain", check_clean_heap_is_clean);
	h_run("E10 compteur et bitmap", check_detects_counter_and_bitmap);
	h_run("E10 en-tete", check_detects_header_tamper);
	h_run("E10 listes", check_detects_list_tamper);
	return (h_end());
}
