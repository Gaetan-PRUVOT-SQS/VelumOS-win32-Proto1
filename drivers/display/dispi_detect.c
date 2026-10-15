#include "display_int.h"
#include "velum/err.h"

int	dispi_detect(t_dispi *d)
{
	uint16_t	id;
	uint64_t	vram;

	if (!d->ops.read || !d->ops.write)
		return (E_NODEV);
	dispi_wr(d, DISPI_INDEX_ID, DISPI_ID_WRITE);
	id = dispi_rd(d, DISPI_INDEX_ID);
	if (id < DISPI_ID_LFB || id > DISPI_ID_LIMIT)
		return (E_NODEV);
	vram = (uint64_t)dispi_rd(d, DISPI_INDEX_VRAM64K) * DISPI_VRAM_UNIT;
	if (!vram || (d->bar_bytes && vram > d->bar_bytes))
		vram = d->bar_bytes;
	if (!vram)
		return (E_NODEV);
	d->vram = vram;
	d->ready = true;
	return (E_OK);
}

bool	dispi_pci_match(const t_pcidev *dev, uint64_t fb_phys)
{
	if (!dev || (dev->bar_flags[0] & PCI_BAR_IO))
		return (false);
	if (!dev->bar_size[0] || !fb_phys)
		return (false);
	return (dev->bar[0] == fb_phys);
}
