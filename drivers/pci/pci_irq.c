#include "pci_int.h"
#include "velum/err.h"
#include "velum/irq.h"

static void	cmd_intx(const t_pciloc *l, bool disable)
{
	uint16_t	cmd;

	cmd = (uint16_t)pcicfg_read(l, PCI_REG_COMMAND, 2);
	if (disable)
		cmd |= PCI_CMD_INTX_OFF;
	else
		cmd &= ~PCI_CMD_INTX_OFF;
	pcicfg_write(l, PCI_REG_COMMAND, 2, cmd);
}

static int	msi_setup(const t_pcidev *d, t_pcipriv *p, t_irqfn fn, void *ctx)
{
	t_pciloc	l;
	t_msimsg	m;
	int			vec;
	int			rc;

	vec = irq_vector_alloc(fn, ctx);
	if (vec < 0)
		return (vec);
	pcidev_loc(d, &l);
	rc = pci_msi_encode(apic_id(), vec, &m);
	if (rc == 0)
		rc = pcimsi_enable(&l, p->msi_off, &m);
	if (rc < 0)
	{
		irq_vector_free(vec);
		return (rc);
	}
	cmd_intx(&l, true);
	p->irq_mode = PCI_IRQ_MSI;
	p->vector = vec;
	return (0);
}

static int	intx_setup(const t_pcidev *d, t_pcipriv *p, t_irqfn fn, void *ctx)
{
	t_pciloc	l;
	uint32_t	gsi;
	int			rc;

	if (!d->irq_pin || d->irq_pin > 4 || !d->irq_line
		|| d->irq_line == PCI_IRQ_LINE_NONE)
		return (E_NODEV);
	gsi = d->irq_line;
	if (gsi < 16)
		gsi = irq_isa_to_gsi((uint8_t)gsi);
	rc = irq_request(gsi, fn, ctx, IRQF_SHARED | IRQF_LEVEL | IRQF_LOW);
	if (rc < 0)
		return (rc);
	pcidev_loc(d, &l);
	cmd_intx(&l, false);
	p->irq_mode = PCI_IRQ_INTX;
	p->gsi = gsi;
	p->fn = fn;
	return (0);
}

int	pci_irq_setup(const t_pcidev *d, t_irqfn fn, void *ctx)
{
	t_pcistate	*st;
	uint64_t	flags;
	int			rc;
	int			idx;

	idx = pcidev_index(d);
	if (idx < 0 || !fn)
		return (E_INVAL);
	st = pci_state();
	flags = spin_lock_irqsave(&st->lock);
	rc = E_BUSY;
	if (st->priv[idx].irq_mode == PCI_IRQ_NONE)
	{
		rc = E_NODEV;
		if (d->has_msi)
			rc = msi_setup(d, &st->priv[idx], fn, ctx);
		if (rc < 0)
			rc = intx_setup(d, &st->priv[idx], fn, ctx);
	}
	spin_unlock_irqrestore(&st->lock, flags);
	return (rc);
}

void	pci_irq_free(const t_pcidev *d)
{
	t_pcistate	*st;
	t_pcipriv	*p;
	t_pciloc	l;
	uint64_t	flags;

	if (pcidev_index(d) < 0)
		return ;
	st = pci_state();
	p = &st->priv[pcidev_index(d)];
	pcidev_loc(d, &l);
	flags = spin_lock_irqsave(&st->lock);
	if (p->irq_mode == PCI_IRQ_MSI)
	{
		pcimsi_disable(&l, p->msi_off);
		irq_vector_free(p->vector);
	}
	else if (p->irq_mode == PCI_IRQ_INTX)
	{
		cmd_intx(&l, true);
		irq_free(p->gsi, p->fn);
	}
	p->irq_mode = PCI_IRQ_NONE;
	spin_unlock_irqrestore(&st->lock, flags);
}
