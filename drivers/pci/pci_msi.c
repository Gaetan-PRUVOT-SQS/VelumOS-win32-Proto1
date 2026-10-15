#include "pci_int.h"
#include "velum/err.h"

int	pci_msi_encode(uint32_t apic, int vec, t_msimsg *msg)
{
	if (vec < PCI_VEC_MIN || vec > PCI_VEC_MAX)
		return (E_INVAL);
	if (apic > 0xff)
		return (E_RANGE);
	msg->addr_lo = PCI_MSI_ADDR_BASE | (apic << 12);
	msg->addr_hi = 0;
	msg->data = (uint16_t)vec;
	return (0);
}

int	pcimsi_enable(const t_pciloc *l, uint16_t cap, const t_msimsg *m)
{
	uint16_t	ctrl;
	uint16_t	data_off;

	ctrl = (uint16_t)pcicfg_read(l, cap + PCI_MSI_CTRL, 2);
	ctrl &= ~(PCI_MSI_ENABLE | PCI_MSI_MME_MASK);
	pcicfg_write(l, cap + PCI_MSI_CTRL, 2, ctrl);
	pcicfg_write(l, cap + PCI_MSI_ADDR, 4, m->addr_lo);
	data_off = cap + 8;
	if (ctrl & PCI_MSI_64BIT)
	{
		pcicfg_write(l, cap + 8, 4, m->addr_hi);
		data_off = cap + 12;
	}
	pcicfg_write(l, data_off, 2, m->data);
	if (ctrl & PCI_MSI_PVM)
		pcicfg_write(l, data_off + 4, 4, 0);
	pcicfg_write(l, cap + PCI_MSI_CTRL, 2, ctrl | PCI_MSI_ENABLE);
	if (!(pcicfg_read(l, cap + PCI_MSI_CTRL, 2) & PCI_MSI_ENABLE))
		return (E_IO);
	return (0);
}

void	pcimsi_disable(const t_pciloc *l, uint16_t cap)
{
	uint16_t	ctrl;

	ctrl = (uint16_t)pcicfg_read(l, cap + PCI_MSI_CTRL, 2);
	ctrl &= ~PCI_MSI_ENABLE;
	pcicfg_write(l, cap + PCI_MSI_CTRL, 2, ctrl);
}
