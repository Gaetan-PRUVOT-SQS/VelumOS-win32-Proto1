#include "irq_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "velum/err.h"

int	irq_vector_alloc(t_irqfn fn, void *ctx)
{
	t_irq_state	*st;
	uint64_t	fl;
	int			idx;
	int			rc;

	st = irq_state();
	if (!fn || !st->ready)
		return (E_INVAL);
	fl = a05_lock(&st->lock);
	idx = irqc_vec_alloc(&st->core);
	rc = idx;
	if (idx >= 0)
	{
		rc = irqc_attach(&st->core, idx, fn, ctx);
		if (rc < 0)
			irqc_vec_release(&st->core, idx);
	}
	a05_unlock(&st->lock, fl);
	if (rc < 0)
		return (rc);
	return (VEC_IRQ_BASE + idx);
}

void	irq_vector_free(int vec)
{
	t_irq_state	*st;
	t_irq_slot	*s;
	uint64_t	fl;
	int			idx;

	st = irq_state();
	idx = vec - VEC_IRQ_BASE;
	if (idx < 0 || idx >= IRQ_NVEC)
		return ;
	fl = a05_lock(&st->lock);
	s = &st->core.slot[idx];
	if (s->used && !s->foreign && s->gsi == IRQ_NO_GSI)
		irqc_vec_release(&st->core, idx);
	a05_unlock(&st->lock, fl);
}

void	irq_mask(uint32_t gsi)
{
	ioapic_set_mask(gsi, true);
}

void	irq_unmask(uint32_t gsi)
{
	ioapic_set_mask(gsi, false);
}
