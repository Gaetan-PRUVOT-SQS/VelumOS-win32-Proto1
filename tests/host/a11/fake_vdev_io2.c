#include <stdlib.h>
#include "fake.h"
#include "virtio.h"

t_fkdev	*fk_by_addr(volatile uint8_t *a, uint32_t *off)
{
	int			i;
	uintptr_t	p;
	uintptr_t	b;

	p = (uintptr_t)a;
	i = 0;
	while (i < FK_DEVS)
	{
		b = (uintptr_t)g_fkdev[i].bar;
		if (g_fkdev[i].present && b && p >= b && p < b + FK_BAR_SIZE)
		{
			*off = (uint32_t)(p - b);
			return (&g_fkdev[i]);
		}
		i++;
	}
	abort();
}

void	vio_wr32(volatile uint8_t *base, uint32_t off, uint32_t v)
{
	t_fkdev		*f;
	uint32_t	o;

	f = fk_by_addr(base + off, &o);
	fk_region_wr(f, o, v);
}

void	vio_wr64(volatile uint8_t *base, uint32_t off, uint64_t v)
{
	vio_wr32(base, off, (uint32_t)v);
	vio_wr32(base, off + 4, (uint32_t)(v >> 32));
}

void	pci_cfg_write32(const t_pcidev *d, uint16_t off, uint32_t v)
{
	int	i;

	i = 0;
	while (i < FK_DEVS && &g_fkdev[i].pci != d)
		i++;
	if (i == FK_DEVS || off > 252 || (off & 3) || (off == 4 && (v >> 16)))
		abort();
	fk_le32(g_fkdev[i].cfg + off, v);
	if (off == 4)
		g_fkdev[i].raw_cmd_writes++;
	if (off == 4)
		g_fkdev[i].cmd = (uint16_t)v;
}
