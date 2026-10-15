#include "pci_int.h"
#include "velum/klog.h"
#include "velum/libk.h"

static void	scan_bus(t_pciscan *s, uint8_t bus, uint8_t depth);

static void	scan_bridge(t_pciscan *s, const t_pciloc *l, uint8_t depth)
{
	uint32_t	buses;
	uint8_t		secondary;
	uint8_t		subordinate;

	buses = pcicfg_read(l, PCI_REG_BUSES, 4);
	secondary = (uint8_t)(buses >> 8);
	subordinate = (uint8_t)(buses >> 16);
	if (secondary <= l->bus || subordinate < secondary
		|| depth >= PCI_MAX_DEPTH)
	{
		klog_debug("pci: pont %02x:%02x.%u sans bus secondaire utilisable",
			l->bus, l->dev, l->fn);
		return ;
	}
	if (!(s->seen[secondary / 8] & (1u << (secondary % 8))))
		scan_bus(s, secondary, (uint8_t)(depth + 1));
}

static void	scan_function(t_pciscan *s, const t_pciloc *l, uint8_t depth)
{
	const t_pcidev	*d;
	uint8_t			htype;
	int				idx;

	htype = (uint8_t)pcicfg_read(l, PCI_REG_HEADER, 1) & PCI_HDR_TYPE_MASK;
	idx = pcidev_add(l, htype);
	if (idx < 0)
		return ;
	d = &pci_state()->devs[idx];
	if (htype == PCI_HDR_BRIDGE && d->class_code == PCI_CLASS_BRIDGE
		&& d->subclass == PCI_SUBCLASS_PCI_BRIDGE)
		scan_bridge(s, l, depth);
}

static void	scan_slot(t_pciscan *s, t_pciloc *l, uint8_t depth)
{
	uint8_t	count;
	uint8_t	fn;

	l->fn = 0;
	if (pcicfg_probe(l, NULL) != 0)
		return ;
	count = 1;
	if (pcicfg_read(l, PCI_REG_HEADER, 1) & PCI_HDR_MULTIFN)
		count = PCI_FNS_PER_DEV;
	fn = 0;
	while (fn < count)
	{
		l->fn = fn;
		if (fn == 0 || pcicfg_probe(l, NULL) == 0)
			scan_function(s, l, depth);
		fn++;
	}
}

static void	scan_bus(t_pciscan *s, uint8_t bus, uint8_t depth)
{
	t_pciloc	l;
	uint8_t		dev;

	s->seen[bus / 8] |= (uint8_t)(1u << (bus % 8));
	l.seg = s->seg;
	l.bus = bus;
	l.fn = 0;
	dev = 0;
	while (dev < PCI_DEVS_PER_BUS)
	{
		l.dev = dev;
		scan_slot(s, &l, depth);
		dev++;
	}
}

void	pci_scan_root(uint16_t seg, uint8_t bus)
{
	t_pciscan	s;

	s.seg = seg;
	memset(s.seen, 0, sizeof(s.seen));
	scan_bus(&s, bus, 0);
}
