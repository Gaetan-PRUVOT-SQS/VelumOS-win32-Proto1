#include "pci_int.h"
#include "velum/boot.h"
#include "velum/klog.h"

static int	q35_devices(void)
{
	const t_pcidev	*vga;

	if (!pci_find(0x8086, 0x29c0, 0))
		return (7);
	vga = pci_find(0x1234, 0x1111, 0);
	if (!vga)
		return (8);
	if (vga->bar_size[0] < 0x400000 || !(vga->bar_flags[0] & PCI_BAR_PREFETCH))
		return (9);
	return (0);
}

int	pci_selftest(void)
{
	int	rc;

	rc = pci_selftest_devices();
	if (!rc && boot_cmdline_has("pci.q35"))
		rc = q35_devices();
	if (!rc && boot_cmdline_has("pci.edu"))
		rc = pci_selftest_edu();
	if (rc)
		klog_err("pci: controle %d en echec", rc);
	return (rc);
}
