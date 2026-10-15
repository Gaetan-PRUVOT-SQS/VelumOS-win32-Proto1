#include "pci_int.h"
#include "velum/err.h"
#include "velum/irqflags.h"
#include "velum/libk.h"

static uint32_t	bar_sized(const t_pciloc *l, uint16_t off, uint32_t *orig)
{
	uint32_t	sized;

	*orig = pcicfg_read(l, off, 4);
	pcicfg_write(l, off, 4, 0xffffffffu);
	sized = pcicfg_read(l, off, 4);
	pcicfg_write(l, off, 4, *orig);
	return (sized);
}

int	pcibar_probe(const t_pciloc *l, uint8_t i, uint8_t n, t_pcibar *b)
{
	uint32_t		v[2];
	uint32_t		hi[2];
	const uint32_t	*hp;
	uint16_t		off;
	bool			is64;

	memset(b, 0, sizeof(*b));
	b->slots = 1;
	off = (uint16_t)(PCI_REG_BAR0 + i * 4);
	v[1] = bar_sized(l, off, &v[0]);
	if (v[0] == 0xffffffffu && v[1] == 0xffffffffu)
		return (E_NODEV);
	is64 = !(v[0] & 1) && ((v[0] >> 1) & 3) == 2;
	hp = NULL;
	if (is64 && i + 1 < n)
	{
		hi[1] = bar_sized(l, off + 4, &hi[0]);
		hp = hi;
	}
	pcibar_decode(v, hp, b);
	if (!b->size)
		return (E_NODEV);
	return (0);
}

int	pcibar_scan(const t_pciloc *l, uint8_t nbars, t_pcidev *d)
{
	uint64_t	irqs;
	uint16_t	cmd;
	uint8_t		i;
	t_pcibar	b;

	irqs = irq_save();
	cmd = (uint16_t)pcicfg_read(l, PCI_REG_COMMAND, 2);
	pcicfg_write(l, PCI_REG_COMMAND, 2, cmd & ~PCI_CMD_DECODE);
	i = 0;
	while (i < nbars)
	{
		if (pcibar_probe(l, i, nbars, &b) == 0)
		{
			d->bar[i] = b.base;
			d->bar_size[i] = b.size;
			d->bar_flags[i] = b.flags;
		}
		i = (uint8_t)(i + b.slots);
	}
	pcicfg_write(l, PCI_REG_COMMAND, 2, cmd);
	irq_restore(irqs);
	return (0);
}
