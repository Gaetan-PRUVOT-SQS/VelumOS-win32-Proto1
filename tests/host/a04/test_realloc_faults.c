#include <stdint.h>
#include "a04_fake.h"

static void	realloc_of_freed_small_block(void)
{
	void	*p;
	t_call	call;

	a04_fresh();
	p = kmalloc(40);
	kfree(p);
	call.p = p;
	call.n = 80;
	a04_expect_panic(a04_do_realloc, &call, "objet déjà libéré");
	a04_drain("E3 realloc d'un objet libere");
}

static void	realloc_of_freed_large_block(void)
{
	void	*p;
	t_call	call;

	a04_fresh();
	p = kmalloc(9000);
	kfree(p);
	call.p = p;
	call.n = 80;
	a04_expect_panic(a04_do_realloc, &call,
		"double libération ou pointeur étranger");
	a04_drain("E3 realloc d'un gros bloc libere");
}

static void	realloc_of_foreign_and_inner_pointers(void)
{
	char	*p;
	int		local;
	t_call	call;

	a04_fresh();
	p = kmalloc(100);
	call.p = &local;
	call.n = 10;
	a04_expect_panic(a04_do_realloc, &call, "pointeur étranger au tas");
	call.p = p + 3;
	a04_expect_panic(a04_do_realloc, &call, "n'est pas un début d'objet");
	kfree(p);
	a04_drain("E3 realloc de pointeurs invalides");
}

int	main(void)
{
	h_begin("a04/realloc-fautes");
	h_run("E3 realloc d'un petit libere", realloc_of_freed_small_block);
	h_run("E3 realloc d'un gros libere", realloc_of_freed_large_block);
	h_run("E3 realloc de pointeurs invalides",
		realloc_of_foreign_and_inner_pointers);
	return (h_end());
}
