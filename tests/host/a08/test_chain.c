#include <stdlib.h>
#include "harness.h"
#include "fake.h"

static void	chain_link(t_object **c, int i)
{
	t_ipcmsg	*m;

	chan_create(&c[2 * i], &c[2 * i + 1]);
	if (i == 0)
		return ;
	m = msg_alloc(0);
	m->objs[0] = c[2 * i - 2];
	m->objs[1] = c[2 * i - 1];
	m->nh = 2;
	chan_write(c[2 * i], m);
}

static void	chain_no_recursion(void)
{
	t_object	**c;
	int			i;
	int			n;

	fk_reset();
	n = 100000;
	c = calloc((size_t)n * 2, sizeof(t_object *));
	if (!c)
		abort();
	i = 0;
	while (i < n)
		chain_link(c, i++);
	h_eq_i64("objets vivants", g_fk.heap_live[HEAP_OBJECT], 2 * n);
	obj_unref(c[2 * n - 2]);
	obj_unref(c[2 * n - 1]);
	free(c);
	h_eq_i64("100000 canaux emboites rendus", fk_heap_total(), 0);
}

int	main(void)
{
	h_begin("a08/chain");
	h_run("chain_no_recursion", chain_no_recursion);
	return (h_end());
}
