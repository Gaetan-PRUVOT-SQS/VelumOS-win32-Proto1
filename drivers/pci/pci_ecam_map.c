#include "pci_int.h"
#include "velum/vmm.h"

static void	*map_lookup(const t_ecam *e, uint16_t seg, uint8_t bus)
{
	uint32_t	i;

	i = 0;
	while (i < e->nmap)
	{
		if (e->map[i].seg == seg && e->map[i].bus == bus)
			return (e->map[i].base);
		i++;
	}
	return (NULL);
}

static void	*map_insert(t_ecam *e, uint16_t seg, uint8_t bus, void *base)
{
	uint64_t	flags;
	void		*found;

	flags = spin_lock_irqsave(&e->lock);
	found = map_lookup(e, seg, bus);
	if (!found && e->nmap < PCI_ECAM_MAP_MAX)
	{
		e->map[e->nmap].seg = seg;
		e->map[e->nmap].bus = bus;
		e->map[e->nmap].base = base;
		e->nmap++;
		found = base;
		base = NULL;
	}
	spin_unlock_irqrestore(&e->lock, flags);
	if (base)
		vmm_io_unmap(base, PCI_ECAM_BUS_LEN);
	return (found);
}

void	*pci_ecam_bus_base(uint16_t seg, uint8_t bus)
{
	t_ecam		*e;
	void		*base;
	uint64_t	flags;
	int			w;

	e = &pci_state()->ecam;
	w = pci_ecam_window(seg, bus);
	if (w < 0)
		return (NULL);
	flags = spin_lock_irqsave(&e->lock);
	base = map_lookup(e, seg, bus);
	spin_unlock_irqrestore(&e->lock, flags);
	if (base)
		return (base);
	base = vmm_io_map(e->win[w].base
			+ ((uint64_t)(bus - e->win[w].bus_start) << 20),
			PCI_ECAM_BUS_LEN, VM_NOCACHE);
	if (!base)
		return (NULL);
	return (map_insert(e, seg, bus, base));
}
