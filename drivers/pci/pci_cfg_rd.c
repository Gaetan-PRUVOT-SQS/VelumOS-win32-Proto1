#include "pci_int.h"

uint32_t	pci_cfg_read32(const t_pcidev *d, uint16_t off)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return (0xffffffffu);
	pcidev_loc(d, &l);
	return (pcicfg_read(&l, off, 4));
}

uint16_t	pci_cfg_read16(const t_pcidev *d, uint16_t off)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return (0xffff);
	pcidev_loc(d, &l);
	return ((uint16_t)pcicfg_read(&l, off, 2));
}

uint8_t	pci_cfg_read8(const t_pcidev *d, uint16_t off)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return (0xff);
	pcidev_loc(d, &l);
	return ((uint8_t)pcicfg_read(&l, off, 1));
}
