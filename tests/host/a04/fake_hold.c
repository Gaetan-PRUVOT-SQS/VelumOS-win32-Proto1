#include "a04_fake.h"

uint32_t	a04_grab(size_t size, uint32_t tag, void **hold, uint32_t max)
{
	uint32_t	n;

	n = 0;
	while (n < max)
	{
		hold[n] = kmalloc_tag(size, (t_heap_tag)tag);
		if (!hold[n])
			return (n);
		n++;
	}
	return (n);
}

void	a04_release(void **hold, uint32_t n)
{
	while (n > 0)
		kfree(hold[--n]);
}

void	a04_arena_setup(t_arena *a, uint64_t *bits, uint64_t slots)
{
	t_arena_cfg	cfg;

	cfg.bits = bits;
	cfg.base = 0x100000;
	cfg.slots = slots;
	cfg.shift = 12;
	cfg.name = "test.arena";
	arena_init(a, &cfg);
}

void	a04_do_trim(void *call)
{
	(void)call;
	heap_trim();
}
