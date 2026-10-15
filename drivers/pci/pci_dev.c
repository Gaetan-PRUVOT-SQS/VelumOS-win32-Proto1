#include "pci_int.h"
#include "velum/err.h"
#include "velum/libk.h"

static void	dev_fill_ids(t_pcidev *d, const t_pciloc *l, uint8_t htype)
{
	uint32_t	v;

	v = pcicfg_read(l, PCI_REG_ID, 4);
	d->vendor = (uint16_t)v;
	d->device = (uint16_t)(v >> 16);
	v = pcicfg_read(l, PCI_REG_CLASS, 4);
	d->revision = (uint8_t)v;
	d->prog_if = (uint8_t)(v >> 8);
	d->subclass = (uint8_t)(v >> 16);
	d->class_code = (uint8_t)(v >> 24);
	v = pcicfg_read(l, PCI_REG_IRQ, 4);
	d->irq_line = (uint8_t)v;
	d->irq_pin = (uint8_t)(v >> 8);
	if (htype != PCI_HDR_NORMAL)
		return ;
	v = pcicfg_read(l, PCI_REG_SUBSYS, 4);
	d->subsys_vendor = (uint16_t)v;
	d->subsys_device = (uint16_t)(v >> 16);
}

static void	dev_fill_caps(t_pcidev *d, t_pcipriv *p, const t_pciloc *l)
{
	int	off;

	off = pcicap_next(l, PCI_CAP_MSI, 0);
	d->has_msi = off > 0;
	if (off > 0)
		p->msi_off = (uint16_t)off;
	off = pcicap_next(l, PCI_CAP_MSIX, 0);
	d->has_msix = off > 0;
	if (off > 0)
		p->msix_off = (uint16_t)off;
	off = pcicap_next(l, PCI_CAP_PCIE, 0);
	if (off > 0)
		p->pcie_off = (uint16_t)off;
}

int	pcidev_find_loc(const t_pciloc *l)
{
	const t_pcistate	*st;
	uint32_t			i;

	st = pci_state();
	i = 0;
	while (i < st->count)
	{
		if (st->devs[i].seg == l->seg && st->devs[i].bus == l->bus
			&& st->devs[i].dev == l->dev && st->devs[i].fn == l->fn)
			return ((int)i);
		i++;
	}
	return (-1);
}

int	pcidev_add(const t_pciloc *l, uint8_t htype)
{
	t_pcistate	*st;
	t_pcidev	*d;

	st = pci_state();
	if (pcidev_find_loc(l) >= 0)
		return (E_EXIST);
	if (st->count >= PCI_MAX_DEVS)
	{
		st->overflow++;
		return (E_NOMEM);
	}
	d = &st->devs[st->count];
	memset(d, 0, sizeof(*d));
	memset(&st->priv[st->count], 0, sizeof(st->priv[0]));
	d->seg = l->seg;
	d->bus = l->bus;
	d->dev = l->dev;
	d->fn = l->fn;
	st->priv[st->count].header_type = htype;
	dev_fill_ids(d, l, htype);
	if (htype <= PCI_HDR_BRIDGE)
		dev_fill_caps(d, &st->priv[st->count], l);
	if (htype <= PCI_HDR_BRIDGE)
		pcibar_scan(l, (uint8_t)(PCI_BARS - 4 * htype), d);
	return ((int)st->count++);
}
