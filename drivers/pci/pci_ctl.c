#include "pci_int.h"
#include "velum/err.h"
#include "velum/util.h"
#include "velum/vmm.h"

int	pci_enable(const t_pcidev *d, uint16_t cmd_bits)
{
	t_pcistate	*st;
	t_pciloc	l;
	uint64_t	flags;
	uint16_t	cmd;

	if (pcidev_index(d) < 0 || (cmd_bits & ~(uint16_t)(PCI_CMD_IO
			| PCI_CMD_MEM | PCI_CMD_MASTER)))
		return (E_INVAL);
	st = pci_state();
	pcidev_loc(d, &l);
	flags = spin_lock_irqsave(&st->lock);
	cmd = (uint16_t)(pcicfg_read(&l, PCI_REG_COMMAND, 2) | cmd_bits);
	pcicfg_write(&l, PCI_REG_COMMAND, 2, cmd);
	cmd = (uint16_t)pcicfg_read(&l, PCI_REG_COMMAND, 2);
	spin_unlock_irqrestore(&st->lock, flags);
	if ((cmd & cmd_bits) != cmd_bits)
		return (E_IO);
	return (0);
}

void	*pci_map_bar(const t_pcidev *d, uint32_t bar)
{
	uint64_t	skew;
	uint8_t		*p;

	if (pcidev_index(d) < 0 || bar >= PCI_BARS || !d->bar_size[bar]
		|| !d->bar[bar] || (d->bar_flags[bar] & PCI_BAR_IO))
		return (NULL);
	if (d->bar_size[bar] > PCI_MAP_MAX
		|| d->bar[bar] + d->bar_size[bar] < d->bar[bar])
		return (NULL);
	skew = d->bar[bar] & (PAGE_SIZE - 1);
	p = vmm_io_map(d->bar[bar] - skew,
			align_up(d->bar_size[bar] + skew, PAGE_SIZE), VM_NOCACHE);
	if (!p)
		return (NULL);
	return (p + skew);
}

int	pci_io_port(const t_pcidev *d, uint32_t bar)
{
	if (pcidev_index(d) < 0 || bar >= PCI_BARS)
		return (E_INVAL);
	if (!(d->bar_flags[bar] & PCI_BAR_IO) || !d->bar_size[bar])
		return (E_INVAL);
	if (!d->bar[bar] || d->bar[bar] + d->bar_size[bar] > 0x10000)
		return (E_INVAL);
	return ((int)d->bar[bar]);
}

int	pci_cap_next(const t_pcidev *d, uint8_t id, uint16_t prev)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return (E_INVAL);
	pcidev_loc(d, &l);
	return (pcicap_next(&l, id, prev));
}
