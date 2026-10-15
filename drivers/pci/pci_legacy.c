#include "pci_int.h"
#include "velum/io.h"
#include "velum/io32.h"

#define PCI_PORT_ADDR 0xcf8
#define PCI_PORT_DATA 0xcfc

static t_spinlock	g_cf8_lock = {0, 0, "pci_cf8"};

static uint32_t	legacy_read(const t_pciloc *l, uint16_t off, uint8_t w)
{
	uint32_t	addr;
	uint32_t	v;
	uint64_t	flags;
	uint16_t	port;

	addr = pci_legacy_addr(l, off);
	if (!addr)
		return (0xffffffffu);
	port = (uint16_t)(PCI_PORT_DATA + (off & 3));
	flags = spin_lock_irqsave(&g_cf8_lock);
	outl(PCI_PORT_ADDR, addr);
	if (w == 1)
		v = inb(port);
	else if (w == 2)
		v = inw(port);
	else
		v = inl(port);
	spin_unlock_irqrestore(&g_cf8_lock, flags);
	return (v);
}

static void	legacy_write(const t_pciloc *l, uint16_t off, uint8_t w, uint32_t v)
{
	uint32_t	addr;
	uint64_t	flags;
	uint16_t	port;

	addr = pci_legacy_addr(l, off);
	if (!addr)
		return ;
	port = (uint16_t)(PCI_PORT_DATA + (off & 3));
	flags = spin_lock_irqsave(&g_cf8_lock);
	outl(PCI_PORT_ADDR, addr);
	if (w == 1)
		outb(port, (uint8_t)v);
	else if (w == 2)
		outw(port, (uint16_t)v);
	else
		outl(port, v);
	spin_unlock_irqrestore(&g_cf8_lock, flags);
}

static bool	legacy_bus_ok(uint16_t seg, uint8_t bus)
{
	(void)bus;
	return (seg == 0);
}

void	pci_legacy_ops(t_pciops *out)
{
	out->read = legacy_read;
	out->write = legacy_write;
	out->bus_ok = legacy_bus_ok;
	out->cfg_size = PCI_LEGACY_CFG_SIZE;
}
