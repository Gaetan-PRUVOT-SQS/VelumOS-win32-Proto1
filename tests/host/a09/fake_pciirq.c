#include "a09_test.h"
#include "fake.h"
#include "fake_fab.h"
#include "pci_int.h"

void	fake_nop_isr(void *ctx)
{
	(void)ctx;
}

void	*fab_msi_device(uint16_t ctrl)
{
	fab_reset();
	fake_reset();
	fab_msi_dev(2, ctrl);
	pci_scan_root(0, 0);
	return ((void *)pci_at(0));
}

void	*fab_plain_device(uint8_t line, uint8_t pin)
{
	static const t_fabspec	spec = {0, 2, 0, 0xabcd, 0x0002, 2, 0, 0};

	fab_reset();
	fake_reset();
	fab_irq(fab_make(&spec), line, pin);
	pci_scan_root(0, 0);
	return ((void *)pci_at(0));
}
