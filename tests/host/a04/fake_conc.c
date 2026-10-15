#include <string.h>
#include "a04_fake.h"
#include "velum/util.h"

static uint32_t	conc_check(const void *p)
{
	const uint32_t	*h;

	h = p;
	if (h[0] < 8)
		return (1);
	return (a04_verify((const char *)p + 8, h[0] - 8, (uint8_t)h[1]) != 0);
}

static void	*conc_make(t_conc *c, uint32_t n, void *old)
{
	uint32_t	*h;
	uint32_t	seed;

	if (old)
		h = krealloc(old, n);
	else
		h = kmalloc_tag(n, (t_heap_tag)(c->id % HEAP_TAGS));
	if (!h)
	{
		c->bad++;
		return (old);
	}
	seed = (uint32_t)a04_rng_next(&c->rng);
	h[0] = n;
	h[1] = seed;
	a04_fill((char *)h + 8, n - 8, (uint8_t)seed);
	return (h);
}

static uint32_t	conc_size(t_rng *r)
{
	if (a04_rng_below(r, 50) == 0)
		return (8 + (uint32_t)a04_rng_below(r, 20000));
	return (8 + (uint32_t)a04_rng_below(r, 3000));
}

void	a04_conc_step(t_conc *c)
{
	uint32_t	j;
	uint32_t	r;
	void		*x;

	j = (uint32_t)a04_rng_below(&c->rng, 64);
	r = (uint32_t)a04_rng_below(&c->rng, 3);
	if (!c->own[j])
		c->own[j] = conc_make(c, conc_size(&c->rng), NULL);
	else if (conc_check(c->own[j]))
		c->bad++;
	else if (r == 0)
		c->own[j] = conc_make(c, conc_size(&c->rng), c->own[j]);
	else
	{
		x = __atomic_exchange_n(&c->shared->slot[a04_rng_below(&c->rng,
					A04_SHARE)], c->own[j], __ATOMIC_ACQ_REL);
		c->own[j] = NULL;
		c->bad += (x && conc_check(x));
		kfree(x);
	}
}
