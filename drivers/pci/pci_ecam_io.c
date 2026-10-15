#include "pci_int.h"

static volatile uint8_t	*ecam_ptr(const t_pciloc *l, uint16_t off)
{
	uint8_t	*base;

	base = pci_ecam_bus_base(l->seg, l->bus);
	if (!base)
		return (NULL);
	return (base + pci_ecam_offset(l, off));
}

static uint32_t	ecam_read(const t_pciloc *l, uint16_t off, uint8_t w)
{
	volatile uint8_t	*p;

	p = ecam_ptr(l, off);
	if (!p)
		return (0xffffffffu);
	if (w == 1)
		return (*p);
	if (w == 2)
		return (*(volatile uint16_t *)p);
	return (*(volatile uint32_t *)p);
}

static void	ecam_write(const t_pciloc *l, uint16_t off, uint8_t w, uint32_t v)
{
	volatile uint8_t	*p;

	p = ecam_ptr(l, off);
	if (!p)
		return ;
	if (w == 1)
		*p = (uint8_t)v;
	else if (w == 2)
		*(volatile uint16_t *)p = (uint16_t)v;
	else
		*(volatile uint32_t *)p = v;
}

static bool	ecam_bus_ok(uint16_t seg, uint8_t bus)
{
	return (pci_ecam_window(seg, bus) >= 0);
}

void	pci_ecam_ops(t_pciops *out)
{
	out->read = ecam_read;
	out->write = ecam_write;
	out->bus_ok = ecam_bus_ok;
	out->cfg_size = PCI_ECAM_CFG_SIZE;
}
