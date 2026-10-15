#include "pci_int.h"

uint32_t	pci_legacy_addr(const t_pciloc *l, uint16_t off)
{
	if (l->seg || l->dev >= PCI_DEVS_PER_BUS || l->fn >= PCI_FNS_PER_DEV)
		return (0);
	if (off >= PCI_LEGACY_CFG_SIZE)
		return (0);
	return (0x80000000u | ((uint32_t)l->bus << 16) | ((uint32_t)l->dev << 11)
		| ((uint32_t)l->fn << 8) | (off & 0xfc));
}
