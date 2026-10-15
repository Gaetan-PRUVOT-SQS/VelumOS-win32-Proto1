#include <string.h>
#include "fake_fab.h"
#include "pci_int.h"

t_fab	g_fab;

void	fab_reset(void)
{
	t_pciops	ops;

	memset(&g_fab, 0, sizeof(g_fab));
	pci_state_reset();
	pci_legacy_ops(&ops);
	pcicfg_set_ops(&ops);
}

t_fabdev	*fab_find(uint16_t seg, uint8_t bus, uint8_t dev, uint8_t fn)
{
	uint32_t	i;

	i = 0;
	while (i < g_fab.ndev)
	{
		if (g_fab.devs[i].seg == seg && g_fab.devs[i].bus == bus
			&& g_fab.devs[i].dev == dev && g_fab.devs[i].fn == fn)
			return (&g_fab.devs[i]);
		i++;
	}
	return (NULL);
}

void	fab_put(t_fabdev *d, uint16_t off, uint8_t w, uint32_t v)
{
	uint8_t	i;

	i = 0;
	while (i < w)
	{
		d->cfg[off + i] = (uint8_t)(v >> (8 * i));
		i++;
	}
}

uint32_t	fab_get(const t_fabdev *d, uint16_t off, uint8_t w)
{
	uint32_t	v;
	uint8_t		i;

	v = 0;
	i = 0;
	while (i < w)
	{
		v |= (uint32_t)d->cfg[off + i] << (8 * i);
		i++;
	}
	return (v);
}

t_fabdev	*fab_make(const t_fabspec *s)
{
	t_fabdev	*d;

	if (g_fab.ndev >= FAB_MAX_DEVS)
		return (NULL);
	d = &g_fab.devs[g_fab.ndev++];
	memset(d, 0, sizeof(*d));
	d->bus = s->bus;
	d->dev = s->dev;
	d->fn = s->fn;
	fab_put(d, 0, 2, s->vendor);
	fab_put(d, 2, 2, s->device);
	fab_put(d, 0x0a, 1, s->sub);
	fab_put(d, 0x0b, 1, s->cls);
	fab_put(d, 0x0e, 1, s->htype);
	return (d);
}
