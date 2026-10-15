#include "pci_int.h"

static int	bars_coherent(const t_pcidev *d)
{
	uint32_t	i;
	uint64_t	size;

	i = 0;
	while (i < PCI_BARS)
	{
		size = d->bar_size[i];
		if (size & (size - 1))
			return (0);
		if (size && d->bar[i] & (size - 1))
			return (0);
		if ((d->bar_flags[i] & PCI_BAR_MEM64) && i + 1 < PCI_BARS
			&& d->bar_size[i + 1])
			return (0);
		i++;
	}
	return (1);
}

static int	same_on_both_paths(const t_pcidev *d)
{
	t_pciops	legacy;
	t_pciloc	l;

	pcidev_loc(d, &l);
	if (l.seg)
		return (1);
	pci_legacy_ops(&legacy);
	return (legacy.read(&l, PCI_REG_ID, 4) == pcicfg_read(&l, PCI_REG_ID, 4));
}

static int	one_device(const t_pcidev *d)
{
	uint32_t	id;

	id = pci_cfg_read32(d, PCI_REG_ID);
	if (d->vendor == 0xffff || id != (((uint32_t)d->device << 16) | d->vendor))
		return (2);
	if (!bars_coherent(d))
		return (3);
	if (d->has_msi != (pci_cap_next(d, PCI_CAP_MSI, 0) > 0))
		return (4);
	if (!same_on_both_paths(d))
		return (5);
	return (0);
}

int	pci_selftest_devices(void)
{
	const t_pcidev	*d;
	uint32_t		i;
	int				rc;

	if (pci_count() == 0)
		return (1);
	i = 0;
	while (i < pci_count())
	{
		d = pci_at(i);
		rc = one_device(d);
		if (rc)
			return (rc);
		i++;
	}
	d = pci_find_class(PCI_CLASS_BRIDGE, 0, 0);
	if (!d || d->bus != 0 || d->dev != 0 || d->fn != 0)
		return (6);
	return (0);
}
