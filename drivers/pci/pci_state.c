#include "pci_int.h"
#include "velum/libk.h"

static t_pcistate	g_pci;

t_pcistate	*pci_state(void)
{
	if (!g_pci.lock.name)
	{
		spin_init(&g_pci.lock, "pci");
		spin_init(&g_pci.ecam.lock, "pci_ecam");
	}
	return (&g_pci);
}

void	pci_state_reset(void)
{
	memset(&g_pci, 0, sizeof(g_pci));
	pci_state();
}

void	pcicfg_set_ops(const t_pciops *ops)
{
	t_pcistate	*st;

	st = pci_state();
	st->ops = *ops;
	st->has_ops = true;
}

int	pcidev_index(const t_pcidev *d)
{
	t_pcistate	*st;

	st = pci_state();
	if (d < st->devs || d >= st->devs + st->count)
		return (-1);
	return ((int)(d - st->devs));
}

void	pcidev_loc(const t_pcidev *d, t_pciloc *l)
{
	l->seg = d->seg;
	l->bus = d->bus;
	l->dev = d->dev;
	l->fn = d->fn;
}
