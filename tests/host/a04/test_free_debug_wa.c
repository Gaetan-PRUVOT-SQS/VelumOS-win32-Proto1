#include <stdint.h>
#include "a04_fake.h"

static void	debug_write_after_free(void)
{
	uint8_t	*p;
	void	*q;
	t_call	call;

	a04_fresh();
	p = kmalloc(24);
	kfree(p);
	p[3] = 7;
	call.n = 24;
	if (HEAP_DEBUG)
		a04_expect_panic(a04_do_malloc, &call, "écriture après libération");
	p[3] = 0xdd;
	if (HEAP_DEBUG)
		kfree(p);
	q = kmalloc(24);
	h_true(q != NULL, "E9 apres reparation");
	kfree(q);
	a04_drain("E9 ecriture apres liberation");
}

int	main(void)
{
	h_begin("a04/debogage-poison");
	h_run("E9 ecriture apres liberation", debug_write_after_free);
	return (h_end());
}
