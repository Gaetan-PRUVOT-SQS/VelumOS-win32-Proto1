#include <stdint.h>
#include "a04_fake.h"

static void	inner_small_misaligned(void)
{
	char	*p;
	char	*q;

	a04_fresh();
	p = kmalloc(100);
	q = kmalloc(100);
	a04_expect_panic(kfree, p + 1, "n'est pas un début d'objet");
	a04_expect_panic(kfree, p + 16, "n'est pas un début d'objet");
	a04_expect_panic(kfree, q - 1, "n'est pas un début d'objet");
	kfree(p);
	kfree(q);
	a04_drain("E3 pointeur interieur petit");
}

static void	inner_slab_header_and_tail(void)
{
	char			*p;
	char			*start;
	const t_class	*c;

	a04_fresh();
	p = kmalloc(100);
	c = &g_heap.cls[slab_class_find(heap_need(100), 16)];
	start = p - c->obj_off;
	a04_expect_panic(kfree, start + 8, "en-tête de dalle");
	a04_expect_panic(kfree, start, "en-tête de dalle");
	a04_expect_panic(kfree, start + c->obj_off + c->nobj * c->size,
		"hors des objets de la dalle");
	kfree(p);
	a04_drain("E3 en-tete et queue de dalle");
}

static void	inner_large_misaligned(void)
{
	char	*p;

	a04_fresh();
	p = kmalloc(10000);
	a04_expect_panic(kfree, p + 16, "n'est pas un début de bloc");
	a04_expect_panic(kfree, p + 1, "n'est pas un début de bloc");
	a04_expect_panic(kfree, p + 4096, "double libération ou pointeur étranger");
	a04_expect_panic(kfree, p - 64, "double libération ou pointeur étranger");
	kfree(p);
	a04_drain("E3 pointeur interieur gros bloc");
}

int	main(void)
{
	h_begin("a04/liberation-interieur");
	h_run("E3 pointeur interieur petit", inner_small_misaligned);
	h_run("E3 en-tete et queue de dalle", inner_slab_header_and_tail);
	h_run("E3 pointeur interieur gros", inner_large_misaligned);
	return (h_end());
}
