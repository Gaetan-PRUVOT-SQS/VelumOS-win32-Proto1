#include "display_int.h"
#include "velum/err.h"
#include "velum/pci.h"

int	dispi_hw_open(t_dispi *d, uint64_t fb_phys)
{
	const t_pcidev	*dev;
	void			*bar;

	if (d->opened)
		return (E_OK);
	dev = pci_find(DISPI_PCI_VENDOR, DISPI_PCI_DEVICE, 0);
	if (!dispi_pci_match(dev, fb_phys))
		return (E_NODEV);
	if (pci_enable(dev, PCI_CMD_MEM | PCI_CMD_IO) < 0)
		return (E_IO);
	bar = pci_map_bar(dev, DISPI_MMIO_BAR);
	if (bar)
		dispi_mmio_bind(d, bar);
	else
		dispi_io_bind(d);
	d->bar_bytes = dev->bar_size[0];
	d->opened = true;
	return (E_OK);
}
