#include <stdlib.h>
#include "fake.h"
#include "velum/err.h"

static t_fkdev	*fk_of(const t_pcidev *d)
{
	int	i;

	i = 0;
	while (i < FK_DEVS)
	{
		if (&g_fkdev[i].pci == d && g_fkdev[i].present)
			return (&g_fkdev[i]);
		i++;
	}
	abort();
}

const t_pcidev	*pci_find(uint16_t vendor, uint16_t device, uint32_t index)
{
	int			i;
	uint32_t	seen;

	i = 0;
	seen = 0;
	while (i < FK_DEVS)
	{
		if (g_fkdev[i].present && g_fkdev[i].pci.vendor == vendor
			&& g_fkdev[i].pci.device == device)
		{
			if (seen == index)
				return (&g_fkdev[i].pci);
			seen++;
		}
		i++;
	}
	return (NULL);
}

uint32_t	pci_cfg_read32(const t_pcidev *d, uint16_t off)
{
	t_fkdev	*f;

	f = fk_of(d);
	if (off > 252 || (off & 3))
		abort();
	return ((uint32_t)f->cfg[off] | ((uint32_t)f->cfg[off + 1] << 8)
		| ((uint32_t)f->cfg[off + 2] << 16)
		| ((uint32_t)f->cfg[off + 3] << 24));
}

int	pci_enable(const t_pcidev *d, uint16_t cmd_bits)
{
	t_fkdev	*f;

	f = fk_of(d);
	if (cmd_bits & ~(uint16_t)(PCI_CMD_IO | PCI_CMD_MEM | PCI_CMD_MASTER))
		return (E_INVAL);
	if (f->modes & FK_ENABLE_FAIL)
		return (E_IO);
	f->cmd |= cmd_bits;
	f->cfg[4] = (uint8_t)f->cmd;
	f->cfg[5] = (uint8_t)(f->cmd >> 8);
	return (0);
}

void	*pci_map_bar(const t_pcidev *d, uint32_t bar)
{
	t_fkdev	*f;

	f = fk_of(d);
	if ((f->modes & FK_MAP_FAIL) || bar != FK_BAR)
		return (NULL);
	return (f->bar);
}
