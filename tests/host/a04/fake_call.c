#include "a04_fake.h"

void	a04_do_malloc(void *call)
{
	t_call	*c;

	c = call;
	c->p = kmalloc(c->n);
}

void	a04_do_realloc(void *call)
{
	t_call	*c;

	c = call;
	c->p = krealloc(c->p, c->n);
}

void	a04_do_give(void *call)
{
	t_give	*g;

	g = call;
	arena_give(g->a, g->idx, g->n);
}

void	a04_do_tag(void *call)
{
	t_tagcall	*c;

	c = call;
	c->out = kmalloc_tag(c->size, (t_heap_tag)c->tag);
}
