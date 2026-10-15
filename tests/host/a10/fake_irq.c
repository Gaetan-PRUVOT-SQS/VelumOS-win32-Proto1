#include <string.h>
#include "fake.h"
#include "velum/err.h"
#include "velum/irq.h"

t_firq	g_firq;

void	firq_reset(void)
{
	uint32_t	i;

	memset(&g_firq, 0, sizeof(g_firq));
	g_firq.fail_gsi = 0xffffffffu;
	i = 0;
	while (i < 16)
	{
		g_firq.gsi_of_isa[i] = i;
		i++;
	}
}

uint32_t	irq_isa_to_gsi(uint8_t isa)
{
	return (g_firq.gsi_of_isa[isa & 15]);
}

int	irq_request(uint32_t gsi, t_irqfn fn, void *ctx, uint32_t flags)
{
	t_firqreg	*r;

	if (gsi == g_firq.fail_gsi)
		return (E_BUSY);
	if (g_firq.count >= FIRQ_MAX)
		return (E_NOMEM);
	r = &g_firq.reg[g_firq.count];
	r->gsi = gsi;
	r->fn = fn;
	r->ctx = ctx;
	r->flags = flags;
	g_firq.count++;
	return (0);
}
