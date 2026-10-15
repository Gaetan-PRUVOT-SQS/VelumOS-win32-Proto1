#include "pci_int.h"
#include "velum/err.h"

static uint32_t	width_mask(uint8_t width)
{
	if (width == 1)
		return (0xff);
	if (width == 2)
		return (0xffff);
	if (width == 4)
		return (0xffffffffu);
	return (0);
}

static bool	args_ok(const t_pciloc *l, uint16_t off, uint8_t width)
{
	const t_pciops	*ops;

	ops = &pci_state()->ops;
	if (!pci_state()->has_ops || !l || !width_mask(width))
		return (false);
	if (l->dev >= PCI_DEVS_PER_BUS || l->fn >= PCI_FNS_PER_DEV)
		return (false);
	if (off % width || off + width > ops->cfg_size)
		return (false);
	return (ops->bus_ok(l->seg, l->bus));
}

uint32_t	pcicfg_read(const t_pciloc *l, uint16_t off, uint8_t width)
{
	const t_pciops	*ops;
	uint32_t		mask;

	mask = width_mask(width);
	if (!mask)
		return (0xffffffffu);
	if (!args_ok(l, off, width))
		return (mask);
	ops = &pci_state()->ops;
	return (ops->read(l, off, width) & mask);
}

int	pcicfg_write(const t_pciloc *l, uint16_t off, uint8_t width, uint32_t v)
{
	const t_pciops	*ops;

	if (!args_ok(l, off, width))
		return (E_INVAL);
	ops = &pci_state()->ops;
	ops->write(l, off, width, v & width_mask(width));
	return (0);
}

int	pcicfg_probe(const t_pciloc *l, uint32_t *id)
{
	uint32_t	v;

	if (!args_ok(l, PCI_REG_ID, 4))
		return (E_INVAL);
	v = pci_state()->ops.read(l, PCI_REG_ID, 4);
	if ((v & 0xffff) == 0xffff || (v & 0xffff) == 0)
		return (E_NODEV);
	if (id)
		*id = v;
	return (0);
}
