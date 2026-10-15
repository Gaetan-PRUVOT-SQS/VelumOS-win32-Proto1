#include <stdint.h>
#include "a04_fake.h"

static void	debug_fill_patterns(void)
{
	uint8_t	*p;

	a04_fresh();
	p = kmalloc(24);
	if (HEAP_DEBUG)
		h_true(p[0] == 0xa5 && p[23] == 0xa5, "E9 alloc : remplissage 0xA5");
	else
		h_true(p[0] == A04_DIRTY, "E9 release : aucun remplissage");
	kfree(p);
	if (HEAP_DEBUG)
		h_true(p[0] == 0xdd && p[31] == 0xdd, "E9 free : poison 0xDD");
	a04_drain("E9 motifs");
}

static void	tamper_and_free(uint8_t *p, size_t at, const char *want)
{
	uint8_t	saved;

	saved = p[at];
	p[at] = (uint8_t)(saved ^ 1);
	if (HEAP_DEBUG)
		a04_expect_panic(kfree, p, want);
	p[at] = saved;
	kfree(p);
}

static void	debug_overflow_small(void)
{
	a04_fresh();
	tamper_and_free(kmalloc(100), 100, "débordement de la zone rouge");
	tamper_and_free(kmalloc(100), 111, "débordement de la zone rouge");
	tamper_and_free(kmalloc(100), 127, "débordement de la zone rouge");
	tamper_and_free(kmalloc(100), 120, "débordement de la zone rouge");
	a04_drain("E9 debordement petit");
}

static void	debug_overflow_large(void)
{
	a04_fresh();
	tamper_and_free(kmalloc(5000), 5000, "zone rouge du bloc");
	tamper_and_free(kmalloc(4033), 4033, "zone rouge du bloc");
	a04_drain("E9 debordement gros");
}

int	main(void)
{
	h_begin("a04/debogage");
	h_run("E9 motifs de remplissage", debug_fill_patterns);
	h_run("E9 debordement d'un petit objet", debug_overflow_small);
	h_run("E9 debordement d'un gros bloc", debug_overflow_large);
	return (h_end());
}
