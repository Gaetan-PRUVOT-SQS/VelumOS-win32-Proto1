#include "irq_int.h"
#include "../../arch/x86_64/apic/apic_int.h"
#include "../../arch/x86_64/acpi/acpi_int.h"
#include "velum/err.h"

bool	irq_nmi_reserved(uint32_t gsi)
{
	const t_acpi_extra	*x;
	uint32_t			k;

	x = acpi_extra();
	k = 0;
	while (k < x->nnmi_src && k < ACPI_MAX_NMI)
	{
		if (x->nmi_src_gsi[k] == gsi)
			return (true);
		k++;
	}
	return (false);
}

uint32_t	irq_isa_to_gsi(uint8_t isa)
{
	return (irq_isa_lookup(acpi_info(), isa));
}

static int	line_route(t_irq_state *st, int idx, const t_irq_req *rq)
{
	int	rc;

	rc = ioapic_route(rq->gsi, (uint8_t)(VEC_IRQ_BASE + idx), rq->trig,
			apic_id());
	if (rc < 0)
		irqc_vec_release(&st->core, idx);
	return (rc);
}

int	irq_request(uint32_t gsi, t_irqfn fn, void *ctx, uint32_t flags)
{
	t_irq_state	*st;
	t_irq_req	rq;
	bool		is_new;
	uint64_t	fl;
	int			idx;

	st = irq_state();
	if (!fn || (flags & ~(uint32_t)IRQF_ALL) || !st->ready
		|| !ioapic_covers(gsi) || irq_nmi_reserved(gsi))
		return (E_INVAL);
	rq.gsi = gsi;
	rq.trig = irq_resolve_trig(acpi_info(), gsi, flags);
	rq.fn = fn;
	rq.ctx = ctx;
	rq.shared = (flags & IRQF_SHARED) != 0;
	fl = a05_lock(&st->lock);
	idx = irqc_request(&st->core, &rq, &is_new);
	if (idx >= 0 && is_new)
		idx = line_route(st, idx, &rq);
	a05_unlock(&st->lock, fl);
	if (idx < 0)
		return (idx);
	return (E_OK);
}

void	irq_free(uint32_t gsi, t_irqfn fn)
{
	t_irq_state	*st;
	uint64_t	fl;
	int			idx;

	st = irq_state();
	fl = a05_lock(&st->lock);
	idx = irqc_find_gsi(&st->core, gsi);
	if (idx >= 0 && irqc_detach(&st->core, idx, fn) == 0)
	{
		ioapic_set_mask(gsi, true);
		irqc_vec_release(&st->core, idx);
	}
	a05_unlock(&st->lock, fl);
}
