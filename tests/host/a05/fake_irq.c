#include "fake.h"
#include "irq_int.h"

void	ffn_a(void *ctx)
{
	(void)ctx;
}

void	ffn_b(void *ctx)
{
	(void)ctx;
}

void	ffn_c(void *ctx)
{
	(void)ctx;
}

int	firq_req(t_irq_core *c, uint32_t gsi, t_irqfn fn, uint32_t flags)
{
	t_irq_req	rq;
	bool		is_new;
	int			rc;

	rq.gsi = gsi;
	rq.fn = fn;
	rq.ctx = NULL;
	rq.trig = flags & IRQF_TRIG_MASK;
	rq.shared = (flags & IRQF_SHARED) != 0;
	rc = irqc_request(c, &rq, &is_new);
	if (rc >= 0 && !is_new)
		return (1000 + rc);
	return (rc);
}
