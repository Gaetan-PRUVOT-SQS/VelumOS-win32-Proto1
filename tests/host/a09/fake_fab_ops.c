#include "fake_fab.h"
#include "pci_int.h"

static uint32_t	fab_read(const t_pciloc *l, uint16_t off, uint8_t w)
{
	const t_fabdev	*d;

	g_fab.reads++;
	d = fab_find(l->seg, l->bus, l->dev, l->fn);
	if (!d)
		return (0xffffffffu);
	if (fab_in_bars(d, off))
		return (fab_bar_reg(d, (uint8_t)((off - 0x10) / 4)) >> ((off & 3) * 8));
	return (fab_get(d, off, w));
}

static uint8_t	wmask(const t_fabdev *d, uint16_t o)
{
	if (o == 0x04 && !d->cmd_ro)
		return (0x47);
	if (o == 0x05 && !d->cmd_ro)
		return (0x05);
	if (o == 0x3c || (o >= 0x18 && o <= 0x1a))
		return (0xff);
	if (d->msi_off && o == d->msi_off + 2u && d->msi_ro)
		return (0x70);
	if (d->msi_off && o == d->msi_off + 2u)
		return (0x71);
	if (d->msi_off && o >= d->msi_off + 4u && o < d->msi_off + 0x14u)
		return (0xff);
	return (0);
}

static void	fab_write(const t_pciloc *l, uint16_t off, uint8_t w, uint32_t v)
{
	t_fabdev	*d;
	uint8_t		i;
	uint8_t		m;

	if (g_fab.nlog < FAB_LOG_MAX)
		g_fab.log[g_fab.nlog++] = (t_fabwr){l->bus, l->dev, l->fn, w, off, v};
	d = fab_find(l->seg, l->bus, l->dev, l->fn);
	if (d && fab_in_bars(d, off))
	{
		g_fab.decode_violations += (v == 0xffffffffu && (d->cfg[4] & 3));
		fab_bar_store(d, (uint8_t)((off - 0x10) / 4), v);
	}
	i = 0;
	while (d && !fab_in_bars(d, off) && i < w)
	{
		m = wmask(d, off + i);
		d->cfg[off + i] = (uint8_t)((d->cfg[off + i] & ~m)
				| ((v >> (8 * i)) & m));
		i++;
	}
}

static bool	fab_bus_ok(uint16_t seg, uint8_t bus)
{
	(void)bus;
	return (seg == 0);
}

void	pci_legacy_ops(t_pciops *out)
{
	out->read = fab_read;
	out->write = fab_write;
	out->bus_ok = fab_bus_ok;
	out->cfg_size = PCI_LEGACY_CFG_SIZE;
}
