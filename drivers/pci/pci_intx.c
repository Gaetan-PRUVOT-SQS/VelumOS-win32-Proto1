#include "pci_int.h"
#include "velum/err.h"

static int	intx_off_locked(const t_pciloc *l)
{
	uint16_t	cmd;
	int			rc;

	cmd = (uint16_t)pcicfg_read(l, PCI_REG_COMMAND, 2);
	if (cmd == 0xffff)
		return (E_NODEV);
	if (cmd & PCI_CMD_INTX_OFF)
		return (0);
	rc = pcicfg_write(l, PCI_REG_COMMAND, 2, cmd | PCI_CMD_INTX_OFF);
	if (rc < 0)
		return (rc);
	cmd = (uint16_t)pcicfg_read(l, PCI_REG_COMMAND, 2);
	if (!(cmd & PCI_CMD_INTX_OFF))
		return (E_NOTSUP);
	return (0);
}

int	pci_intx_disable(const t_pcidev *d)
{
	t_pcistate	*st;
	t_pciloc	l;
	uint64_t	flags;
	int			rc;

	if (pcidev_index(d) < 0)
		return (E_INVAL);
	st = pci_state();
	pcidev_loc(d, &l);
	flags = spin_lock_irqsave(&st->lock);
	rc = intx_off_locked(&l);
	spin_unlock_irqrestore(&st->lock, flags);
	return (rc);
}
