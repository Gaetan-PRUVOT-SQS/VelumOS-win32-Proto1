#include <stdint.h>
#include "a04_fake.h"

static void	faults_double_free_small(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(24);
	kfree(p);
	a04_expect_panic(kfree, p, "double libération");
	a04_drain("E3 double liberation petite");
}

static void	faults_double_free_after_slab_release(void)
{
	void	*p[600];
	size_t	i;

	a04_fresh();
	i = 0;
	while (i < 600)
		p[i++] = kmalloc(16);
	i = 0;
	while (i < 600)
		kfree(p[i++]);
	h_true(g_heap.cls[0].cache[0].slabs == 1, "E3 une dalle gardee");
	heap_trim();
	a04_expect_panic(kfree, p[0], "double libération ou pointeur étranger");
	a04_drain("E3 double liberation apres rendu de la dalle");
}

static void	faults_double_free_large(void)
{
	void	*p;

	a04_fresh();
	p = kmalloc(10000);
	kfree(p);
	a04_expect_panic(kfree, p, "double libération ou pointeur étranger");
	a04_drain("E3 double liberation gros bloc");
}

static void	faults_foreign_pointers(void)
{
	int	local;

	a04_fresh();
	a04_expect_panic(kfree, &local, "pointeur étranger au tas");
	a04_expect_panic(kfree, (void *)(g_heap.lay.base - 16),
		"pointeur étranger au tas");
	a04_expect_panic(kfree, (void *)(g_heap.arena[3].base
			+ ((uintptr_t)A04_LARGE << 12) + 64), "pointeur étranger au tas");
	a04_expect_panic(kfree, (void *)(g_heap.arena[0].base + 5 * 4096 + 64),
		"double libération ou pointeur étranger");
	a04_expect_panic(kfree, (void *)(g_heap.arena[3].base + 100 * 4096 + 64),
		"double libération ou pointeur étranger");
	a04_drain("E3 pointeurs etrangers");
}

int	main(void)
{
	h_begin("a04/liberation-fautes");
	h_run("E3 double liberation petite", faults_double_free_small);
	h_run("E3 double liberation apres rendu",
		faults_double_free_after_slab_release);
	h_run("E3 double liberation gros bloc", faults_double_free_large);
	h_run("E3 pointeurs etrangers", faults_foreign_pointers);
	return (h_end());
}
