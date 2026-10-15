#include "pci_int.h"

void	pci_cfg_write32(const t_pcidev *d, uint16_t off, uint32_t v)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return ;
	pcidev_loc(d, &l);
	pcicfg_write(&l, off, 4, v);
}

void	pci_cfg_write16(const t_pcidev *d, uint16_t off, uint16_t v)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return ;
	pcidev_loc(d, &l);
	pcicfg_write(&l, off, 2, v);
}

void	pci_cfg_write8(const t_pcidev *d, uint16_t off, uint8_t v)
{
	t_pciloc	l;

	if (pcidev_index(d) < 0)
		return ;
	pcidev_loc(d, &l);
	pcicfg_write(&l, off, 1, v);
}
