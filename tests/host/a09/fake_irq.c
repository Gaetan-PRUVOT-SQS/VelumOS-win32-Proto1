#include "fake.h"
#include "velum/irq.h"

int	irq_vector_alloc(t_irqfn fn, void *ctx)
{
	(void)fn;
	(void)ctx;
	if (g_fake.vec_fail < 0)
		return (g_fake.vec_fail);
	g_fake.vec_allocs++;
	return (g_fake.vec_next++);
}

void	irq_vector_free(int vec)
{
	g_fake.vec_frees++;
	g_fake.vec_last_freed = vec;
}

int	irq_request(uint32_t gsi, t_irqfn fn, void *ctx, uint32_t flags)
{
	(void)fn;
	(void)ctx;
	g_fake.irq_reqs++;
	g_fake.irq_req_gsi = gsi;
	g_fake.irq_req_flags = flags;
	return (g_fake.irq_req_rc);
}

void	irq_free(uint32_t gsi, t_irqfn fn)
{
	(void)fn;
	g_fake.irq_frees++;
	g_fake.irq_freed_gsi = gsi;
}

uint32_t	irq_isa_to_gsi(uint8_t isa)
{
	return ((uint32_t)isa + 100);
}
