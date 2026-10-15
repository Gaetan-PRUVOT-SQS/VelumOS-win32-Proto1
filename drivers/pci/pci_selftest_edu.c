#include "pci_int.h"
#include "velum/timer.h"

#define EDU_VENDOR 0x1234
#define EDU_DEVICE 0x11e8
#define EDU_ID 0x010000ed
#define EDU_WAIT_NS 200000000ull

static void	edu_isr(void *ctx)
{
	volatile uint32_t	*hits;

	hits = ctx;
	*hits = *hits + 1;
}

static bool	edu_fired(void *ctx)
{
	volatile uint32_t	*hits;

	hits = ctx;
	return (*hits != 0);
}

static int	edu_registers(volatile uint32_t *regs)
{
	if (regs[0] != EDU_ID)
		return (14);
	regs[1] = 0x12345678;
	if (regs[1] != ~0x12345678u)
		return (15);
	return (0);
}

static int	edu_interrupt(const t_pcidev *d, volatile uint32_t *regs)
{
	volatile uint32_t	hits;
	int					rc;

	hits = 0;
	if (pci_irq_setup(d, edu_isr, (void *)&hits) < 0)
		return (16);
	regs[0x60 / 4] = 1;
	rc = wait_until(edu_fired, (void *)&hits, EDU_WAIT_NS);
	regs[0x64 / 4] = 1;
	pci_irq_free(d);
	if (rc < 0)
		return (17);
	return (0);
}

int	pci_selftest_edu(void)
{
	const t_pcidev		*d;
	volatile uint32_t	*regs;
	int					rc;

	d = pci_find(EDU_VENDOR, EDU_DEVICE, 0);
	if (!d)
		return (10);
	if (!d->has_msi)
		return (11);
	if (pci_enable(d, PCI_CMD_MEM | PCI_CMD_MASTER) < 0)
		return (12);
	regs = pci_map_bar(d, 0);
	if (!regs)
		return (13);
	rc = edu_registers(regs);
	if (!rc)
		rc = edu_interrupt(d, regs);
	return (rc);
}
